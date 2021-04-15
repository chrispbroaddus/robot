using System;
using System.Threading;
using System.Collections.Generic;

namespace Zippy {
    /// <summary>
    /// Producer consumer queue.
    /// Manages a thread pool that run the specified Actions
    /// http://www.albahari.com/threading/part4.aspx#_Signaling_with_Wait_and_Pulse
    /// </summary>
    public class ProducerConsumerQueue {
        readonly object _locker = new object();
        Thread[] _workers;
        Queue<Action> _itemQ = new Queue<Action>();

        /// <summary>
        /// Initializes a new instance of the <see cref="Zippy.ProducerConsumerQueue"/> class woth the specified number of worker threads.
        /// </summary>
        /// <param name="workerCount">Worker count.</param>
        public ProducerConsumerQueue (int workerCount) {
            if (workerCount < 1) {
                workerCount = 1;
            }

            _workers = new Thread [workerCount];

            // Create and start a separate thread for each worker
            for (int i = 0; i < workerCount; i++) {
                (_workers [i] = new Thread (Consume)).Start ();
            }
        }

        /// <summary>
        /// Shutdown the consumers, and optionally wait for the consumer threads
        /// </summary>
        /// <param name="waitForWorkers">Wait for workers.</param>
        public void Shutdown (bool waitForWorkers) {
            // Enqueue one null item per worker to make each exit.
            foreach (Thread worker in _workers) {
                EnqueueItem (null);
            }

            // Wait for workers to finish
            if (waitForWorkers) {
                foreach (Thread worker in _workers) {
                    worker.Join ();
                }
            }
        }

        /// <summary>
        /// Enqueues the item on to the work queue
        /// </summary>
        /// <returns>The item.</returns>
        /// <param name="item">Item.</param>
        public void EnqueueItem (Action item) {
            lock (_locker) {
                _itemQ.Enqueue (item);           // We must pulse because we're
                Monitor.Pulse (_locker);         // changing a blocking condition.
            }
        }

        /// <summary>
        /// Consume an item from the work queue.
        /// </summary>
        void Consume() {
            while (true) {                      // Keep consuming until
                // told otherwise.
                Action item;

                lock (_locker) {
                    while (_itemQ.Count == 0) {
                        Monitor.Wait (_locker);
                    }

                    item = _itemQ.Dequeue();
                }

                if (item == null) {
                    return;    // This signals our exit.
                }

                item();                           // Execute item.
            }
        }
    }
}
