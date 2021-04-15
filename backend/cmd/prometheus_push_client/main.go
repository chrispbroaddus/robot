package main

import (
	"bytes"
	"encoding/json"
	"fmt"
	"io"
	"io/ioutil"
	"log"
	"mime"
	"net/http"
	"time"

	"github.com/alexflint/go-arg"
	"github.com/golang/protobuf/proto"
	"github.com/matttproud/golang_protobuf_extensions/pbutil"
	metrics "github.com/zippyai/zippy/packages/metrics/proto"
)

const (
	segmentDelay = 4 // delay in seconds
)

type pusher struct {
	client  http.Client
	baseURL string
}

type metricsTestPlan struct {
	JobName    string                    `json:"job"`
	Labels     map[string]string         `json:"labels"`
	GatewayURL string                    `json:"url"`
	Families   [][]*metrics.MetricFamily `json:"test_plan_families"`
}

func main() {
	var args struct {
		TestPlan string
	}
	args.TestPlan = "conf/demo_metrics/test_push_metrics.json"
	arg.MustParse(&args)

	testPlan, err := loadTestPlan(args.TestPlan)
	if err != nil {
		log.Fatal(err)
	}

	pusher := &pusher{
		client:  http.Client{},
		baseURL: testPlan.GatewayURL,
	}

	if err = testPlan.execute(pusher); err != nil {
		log.Fatal(err)
	}
}

func (p *pusher) putMetrics(job string, labels map[string]string, family *metrics.MetricFamily) error {
	reqURL := fmt.Sprintf("%s/metrics/job/%s", p.baseURL, job)

	// if labels are passed in they will over write any labels defined in the body
	for key, val := range labels {
		reqURL = fmt.Sprintf("%s/%s/%s", reqURL, key, val)
	}

	reader, err := marshalProto(family)
	if err != nil {
		return err
	}

	req, err := http.NewRequest("POST", reqURL, reader)
	if err != nil {
		return err
	}
	req.Header.Set("Content-Type", mime.FormatMediaType("application/vnd.google.protobuf", map[string]string{
		"proto":    "io.prometheus.client.MetricFamily",
		"encoding": "delimeted",
	}))

	resp, err := p.client.Do(req)
	if err != nil {
		return err
	}

	if resp.StatusCode != 202 {
		blob, _ := ioutil.ReadAll(resp.Body)
		return fmt.Errorf("request failed with status code: %d and error: %s", resp.StatusCode, string(blob))
	}

	return nil
}

func (t *metricsTestPlan) execute(p *pusher) error {
	for i, families := range t.Families {
		log.Printf("starting to push segement %d of %d\n", i+1, len(t.Families))
		for _, family := range families {
			err := p.putMetrics(t.JobName, t.Labels, family)
			if err != nil {
				return err
			}
		}

		if i+2 <= len(t.Families) {
			log.Printf("waiting to start segment: %d will send in %d seconds\n", i+2, segmentDelay)
			time.Sleep(time.Second * segmentDelay)
		}
	}

	return nil
}

func marshalProto(msg proto.Message) (io.Reader, error) {
	var protoBlob []byte
	buffer := bytes.NewBuffer(protoBlob)
	_, err := pbutil.WriteDelimited(buffer, msg)
	if err != nil {
		return nil, err
	}

	return bytes.NewReader(protoBlob), nil
}

func loadTestPlan(filePath string) (*metricsTestPlan, error) {
	fBlob, err := ioutil.ReadFile(filePath)
	if err != nil {
		return nil, err
	}

	testPlan := new(metricsTestPlan)
	if err = json.Unmarshal(fBlob, testPlan); err != nil {
		return nil, err
	}

	return testPlan, nil
}
