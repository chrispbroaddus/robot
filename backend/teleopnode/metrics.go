package teleopnode

import (
	"net/http"
	"strconv"
	"sync"
	"time"

	metrics "github.com/go-kit/kit/metrics/prometheus"
	"github.com/prometheus/client_golang/prometheus"
	teleopproto "github.com/zippyai/zippy/packages/teleop/proto"
)

const (
	nameSpace = "mission_control"
	subSystem = "teleopnode"
)

// package wide metrics
var (
	// counters
	httpRequestsTotal   *metrics.Counter
	httpErrorTotal      *metrics.Counter
	websocketFramesSent *metrics.Counter
	// gauges
	openWebsockets *metrics.Gauge
	// histograms
	httpRequestTime    *metrics.Histogram
	websocketLifeSpan  *metrics.Histogram
	commandProcessTime *metrics.Histogram

	// timingManager manages the start and stop of cross thread commands
	timingManager *commandTimings
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

	websocketFramesSent = metrics.NewCounterFrom(prometheus.CounterOpts{
		Namespace: nameSpace,
		Subsystem: subSystem,
		Name:      "websocket_frames_sent",
		Help:      "Number of frames sent over the websocket during it's lifetime",
	}, []string{"endpoint", "requester"})

	openWebsockets = metrics.NewGaugeFrom(prometheus.GaugeOpts{
		Namespace: nameSpace,
		Subsystem: subSystem,
		Name:      "websocket_open",
		Help:      "Current number of open websockets",
	}, []string{"requester", "endpoint"})

	httpRequestTime = metrics.NewHistogramFrom(prometheus.HistogramOpts{
		Namespace: nameSpace,
		Subsystem: subSystem,
		Name:      "http_request_time",
		Help:      "Duration of http requests by type and requester",
		Buckets:   prometheus.LinearBuckets(100, 100, 10), // start at 100 ms with 10 buckets 100 ms apart
	}, []string{"endpoint", "requester"})

	websocketLifeSpan = metrics.NewHistogramFrom(prometheus.HistogramOpts{
		Namespace: nameSpace,
		Subsystem: subSystem,
		Name:      "websocket_lifespan",
		Help:      "Lifespan of all websockets",
		Buckets:   prometheus.ExponentialBuckets(50, 120, 10), // start at 50 ms with 10 buckets ending at 60s
	}, []string{"endpoint", "requester"})

	commandProcessTime = metrics.NewHistogramFrom(prometheus.HistogramOpts{
		Namespace: nameSpace,
		Subsystem: subSystem,
		Name:      "command_lifespan",
		Help:      "The amount of time it takes a command from the frontend to make it to the vehicle",
		Buckets:   prometheus.LinearBuckets(10, 10, 30), // start at 10 ms with 30 buckets 10 ms apart
	}, []string{"recipient", "type"})

	timingManager = &commandTimings{
		commandTimes: make(map[string]time.Time),
	}
}

// commandTimings is used to track the start times for every websocket command
type commandTimings struct {
	mu           sync.Mutex
	commandTimes map[string]time.Time
}

func (c *commandTimings) Start(commandID string) {
	c.mu.Lock()
	defer c.mu.Unlock()

	c.commandTimes[commandID] = time.Now()
}

func (c *commandTimings) Finish(commandID string) time.Duration {
	c.mu.Lock()
	defer c.mu.Unlock()

	start, ok := c.commandTimes[commandID]
	if !ok {
		return 0
	}

	delete(c.commandTimes, commandID)
	return time.Since(start)
}

func messageType(msg *teleopproto.BackendMessage) string {
	var msgType string
	switch msg.GetPayload().(type) {
	case *teleopproto.BackendMessage_Exposure:
		msgType = "camera_exposure"
	case *teleopproto.BackendMessage_DockCommand:
		msgType = "dock_command"
	case *teleopproto.BackendMessage_IceCandidate:
		msgType = "ice_candidate"
	case *teleopproto.BackendMessage_Joystick:
		msgType = "joystick"
	case *teleopproto.BackendMessage_PointAndGo:
		msgType = "point_and_go"
	case *teleopproto.BackendMessage_SdpConfirmation:
		msgType = "sdp_confirmation"
	case *teleopproto.BackendMessage_SdpRequest:
		msgType = "sdp_request"
	case *teleopproto.BackendMessage_StopCommand:
		msgType = "stop_command"
	case *teleopproto.BackendMessage_VideoRequest:
		msgType = "video_request"
	case *teleopproto.BackendMessage_ZStage:
		msgType = "z_stage"
	default:
		msgType = "unknown_type"
	}

	return msgType
}

// requester is either the userID of the user that called the endpoint or the sessionID of the vehicle that called the endpoint
func respondError(w http.ResponseWriter, r *http.Request, err error, requester, pathTmpl string, status int) {
	httpRequestsTotal.With("endpoint", pathTmpl, "method", r.Method, "requester", requester, "code", strconv.Itoa(status)).Add(1)
	httpErrorTotal.With("endpoint", pathTmpl, "method", r.Method, "requester", requester).Add(1)
	http.Error(w, err.Error(), status)
}
