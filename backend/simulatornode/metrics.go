package simulatornode

import (
	"net/http"
	"strconv"

	metrics "github.com/go-kit/kit/metrics/prometheus"
	"github.com/prometheus/client_golang/prometheus"
)

const (
	nameSpace = "simulators"
	subSystem = "sim_host"
)

// package wide metrics
var (
	// counters
	httpRequestsTotal *metrics.Counter
	httpErrorTotal    *metrics.Counter
	// histograms
	httpRequestTime *metrics.Histogram
)

func init() {
	httpRequestsTotal = metrics.NewCounterFrom(prometheus.CounterOpts{
		Namespace: nameSpace,
		Subsystem: subSystem,
		Name:      "http_request_total",
		Help:      "Number of http requests and their resulting status code",
	}, []string{"code", "method", "endpoint", "requester"})

	httpErrorTotal = metrics.NewCounterFrom(prometheus.CounterOpts{
		Namespace: nameSpace,
		Subsystem: subSystem,
		Name:      "http_error_total",
		Help:      "Number of http requests that encountered errors",
	}, []string{"endpoint", "method", "requester"})

	httpRequestTime = metrics.NewHistogramFrom(prometheus.HistogramOpts{
		Namespace: nameSpace,
		Subsystem: subSystem,
		Name:      "http_request_time",
		Help:      "Duration of http requests by type and requester",
		Buckets:   prometheus.LinearBuckets(100, 100, 10), // start at 100 ms with 10 buckets 100 ms apart
	}, []string{"endpoint", "requester"})
}

// requester is either the userID of the user that called the endpoint or the sessionID of the vehicle that called the endpoint
func respondError(w http.ResponseWriter, r *http.Request, err error, requester, pathTmpl string, status int) {
	httpRequestsTotal.With("endpoint", pathTmpl, "method", r.Method, "requester", requester, "code", strconv.Itoa(status)).Add(1)
	httpErrorTotal.With("endpoint", pathTmpl, "method", r.Method, "requester", requester).Add(1)
	http.Error(w, err.Error(), status)
}
