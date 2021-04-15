package logger

import (
	"context"
	"errors"

	"github.com/go-kit/kit/metrics"
)

var (
	// ErrWrongType is returned if the logger does not reflect to the right type
	ErrWrongType = errors.New("the metrics logger you requested is the wrong type")
	// ErrUnsupportedType is returned if you try and log for a type that's not supported
	ErrUnsupportedType = errors.New("the type of metric you are logging to is not supported")
	// ErrNoMetric is returned if there is not a defined metric for that name
	ErrNoMetric = errors.New("there is no metric for that name")
	// ErrNoMetricValue is returned if there is no valid value to set for metric
	ErrNoMetricValue = errors.New("there is no metric value defined")
)

// MetricType different types of metrics to log
type MetricType int

const (
	// Counter is the type counter that counts totals
	Counter MetricType = iota
	// Gauge is used to track specific values over time
	Gauge
	// Histogram is used to track repeated observations of the same thing and how long they take
	Histogram
)

// MetricsWriter is used to write a metric
type MetricsWriter interface {
	Write(logger MetricsLogger) error
	StartBackground(ctx context.Context)
}

// MetricsLogger is used to log for any type the way that it needs to
type MetricsLogger interface {
	Log(interface{}) error
	Name() string
	Type() MetricType
}

type counter struct {
	name     string
	addValue float64
	labels   []string
}

// NewCountLog creates a new log definition for counters
func NewCountLog(name string, delta float64, labels ...string) MetricsLogger {
	return &counter{
		name:     name,
		labels:   labels,
		addValue: delta,
	}
}

func (c *counter) Log(logger interface{}) error {
	counterBase, ok := logger.(metrics.Counter)
	if !ok {
		return ErrWrongType
	}

	// add additional labels if they're provided
	if len(c.labels) > 0 {
		counterBase = counterBase.With(c.labels...)
	}

	counterBase.Add(c.addValue)

	return nil
}

func (c *counter) Name() string {
	return c.name
}

func (c *counter) Type() MetricType {
	return Counter
}

type gauge struct {
	name     string
	addValue *float64
	setValue *float64
	labels   []string
}

// NewGaugeLog creates a new log definition for gauges
func NewGaugeLog(name string, value, delta *float64, labels ...string) MetricsLogger {
	return &gauge{
		name:     name,
		addValue: delta,
		setValue: value,
		labels:   labels,
	}
}

func (g *gauge) Log(logger interface{}) error {
	gaugeBase, ok := logger.(metrics.Gauge)
	if !ok {
		return ErrWrongType
	}

	// add additional labels if they're provided
	if len(g.labels) > 0 {
		gaugeBase = gaugeBase.With(g.labels...)
	}

	if g.addValue != nil {
		gaugeBase.Add(*g.addValue)
		return nil
	}

	if g.setValue != nil {
		gaugeBase.Add(*g.setValue)
		return nil
	}

	return ErrNoMetricValue
}

func (g *gauge) Name() string {
	return g.name
}

func (g *gauge) Type() MetricType {
	return Gauge
}

type histogram struct {
	name        string
	observation float64
	labels      []string
}

// NewHistogramLog creates a new log definition for histograms
func NewHistogramLog(name string, observation float64, labels ...string) MetricsLogger {
	return &histogram{
		name:        name,
		observation: observation,
		labels:      labels,
	}
}

func (h *histogram) Log(logger interface{}) error {
	histogramBase, ok := logger.(metrics.Histogram)
	if !ok {
		return ErrWrongType
	}

	if len(h.labels) > 0 {
		histogramBase = histogramBase.With(h.labels...)
	}

	histogramBase.Observe(h.observation)

	return nil
}

func (h *histogram) Name() string {
	return h.name
}

func (h *histogram) Type() MetricType {
	return Histogram
}
