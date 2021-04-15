from flask import Flask, request, abort, jsonify
import Queue
import json
import time

q = Queue.Queue()
app = Flask(__name__)

@app.route('/push', methods=['POST'])
def push():
    q.put(json.dumps(request.json))
    return 'OK'

@app.route('/pull', methods=['GET'])
def pull():
    if q.qsize() > 0:
        return jsonify(q.get())
    return 'OK'

if __name__ == '__main__':
      app.run(host='0.0.0.0', port=80)
