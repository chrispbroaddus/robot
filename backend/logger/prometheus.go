package logger

import (
	"context"
	"runtime"
	"time"

	"github.com/go-kit/kit/metrics"
)

type prometheusLogger struct {
	counters  map[string]metrics.Counter
	gauges    map[string]metrics.Gauge
	histogram map[string]metrics.Histogram
}

// CreatePrometheusLogger returns a new logger
func CreatePrometheusLogger() MetricsWriter {
	promLogger := &prometheusLogger{
		counters:  make(map[string]metrics.Counter),
		gauges:    make(map[string]metrics.Gauge),
		histogram: make(map[string]metrics.Histogram),
	}

	return promLogger
}

func (p *prometheusLogger) Write(logger MetricsLogger) error {
	var ok bool
	var metric interface{}
	switch logger.Type() {
	case Counter:
		metric, ok = p.counters[logger.Name()]
	case Gauge:
		metric, ok = p.gauges[logger.Name()]
	case Histogram:
		metric, ok = p.histogram[logger.Name()]
	default:
		return ErrUnsupportedType
	}

	if !ok {
		return ErrNoMetric
	}

	return logger.Log(metric)
}

func (p *prometheusLogger) StartBackground(ctx context.Context) {
	go func() {
		for {
			select {
			case <-ctx.Done():
				return
			case <-time.Tick(time.Second * 5):
				// handles logging all of the system level metrics for every scrape interval
				numRoutines := float64(runtime.NumGoroutine())
				p.Write(NewGaugeLog("goroutine_count", &numRoutines, nil))

				memstats := new(runtime.MemStats)
				runtime.ReadMemStats(memstats)
			}
		}
	}()
}
