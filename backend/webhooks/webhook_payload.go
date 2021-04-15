package webhooks

import (
	"fmt"
	"net/url"
	"strconv"
)

const (
	baseMapsURL = "https://www.google.com/maps/dir/"
)

// PostmatesDispatch is the full payload that is sent from postmates to describe a delivery
type PostmatesDispatch struct {
	ID       string       `json:"uuid"`
	Dropoff  *DropoffInfo `json:"dropoff_info"`
	Manifest *Manifest    `json:"manifest"`
	Pickup   *PickupInfo  `json:"pickup_info"`
}

// DropoffInfo all of the destination information
type DropoffInfo struct {
	CashOnly     bool    `json:"cash_only"`
	City         string  `json:"city"`
	DisplayName  string  `json:"display_name"`
	Lat          float64 `json:"lat"`
	Lng          float64 `json:"lng"`
	Market       string  `json:"market"`
	MobileNumber string  `json:"mobile_number"`
	Notes        string  `json:"notes"`
	PlaceID      string  `json:"place_id"`
	State        string  `json:"state"`
	AddressOne   string  `json:"street_address_1"`
	AddressTwo   string  `json:"street_address_2"`
	ZipCode      string  `json:"zip_code"`
}

// Manifest is the orders manifest detailing everything in the order
type Manifest struct {
	Currency   string        `json:"currency"`
	TotalPrice float64       `json:"estimated_total_price"`
	Groups     []*Properties `json:"groups"`
	ReadyTime  string        `json:"ready_dt"`
	Reference  string        `json:"reference"`
	Status     string        `json:"status"`
	Summary    string        `json:"summary"`
	Title      string        `json:"title"`
}

// Properties in the Manifest
type Properties struct {
	LineItems []*Order `json:"line_items"`
	Name      string   `json:"name"`
}

// Order is and orders line items and everything related to them
type Order struct {
	BasePrice           float64   `json:"base_price"`
	Category            string    `json:"category_name"`
	Currency            string    `json:"currency"`
	Image               []byte    `json:"img"`
	OptionGroups        []*Option `json:"option_groups"`
	Quantity            int       `json:"quantity"`
	SpecialInstructions string    `json:"special_instructions"`
	SubNotes            []string  `json:"sub_notes"`
	SubOptions          []*Option `json:"sub_options"`
	Text                string    `json:"text"`
}

// Option name and options of values
type Option struct {
	Name    string   `json:"name"`
	Options []string `json:"options"`
}

// PickupInfo information on where to get delivery from
type PickupInfo struct {
	CashOnly     bool    `json:"cash_only"`
	City         string  `json:"city"`
	DisplayName  string  `json:"display_name"`
	Lat          float64 `json:"lat"`
	Lng          float64 `json:"lng"`
	Market       string  `json:"market"`
	MobileNumber string  `json:"mobile_number"`
	Notes        string  `json:"notes"`
	PlaceID      string  `json:"place_uuid"`
	State        string  `json:"state"`
	AddressOne   string  `json:"street_address_1"`
	AddressTwo   string  `json:"street_address_2"`
	ZipCode      string  `json:"zip_code"`
}

// PostMatesSlack is the payload used to display dispatch in slack
type PostMatesSlack struct {
	Title       string        `json:"text"`
	Attachments []*Attachment `json:"attachments"`
}

// Attachment slack attachment
type Attachment struct {
	Title  string   `json:"title"`
	Text   string   `json:"text"`
	Color  string   `json:"color"`
	Fields []*Field `json:"fields,omitempty"`
}

// Field values create a table in the message
type Field struct {
	Title string `json:"title"`
	Value string `json:"value"`
	Short bool   `json:"short"`
}

// CreatePostmatesSlackPayload convert what we get from Postmates to how to display it on slack
func CreatePostmatesSlackPayload(p *PostmatesDispatch) (*PostMatesSlack, error) {
	dest := &Attachment{
		Title: "Delivery Destination",
		Color: "#ff9a00",
		Text:  fmt.Sprintf("%s %s %s, %s, %s", p.Dropoff.AddressOne, p.Dropoff.AddressTwo, p.Dropoff.City, p.Dropoff.State, p.Dropoff.ZipCode),
		Fields: []*Field{
			&Field{
				Title: "Lat",
				Value: strconv.FormatFloat(p.Dropoff.Lat, 'E', -1, 32),
				Short: true,
			},
			&Field{
				Title: "Lng",
				Value: strconv.FormatFloat(p.Dropoff.Lng, 'E', -1, 32),
				Short: true,
			},
			&Field{
				Title: "Name",
				Value: p.Dropoff.DisplayName,
				Short: true,
			},
			&Field{
				Title: "Number",
				Value: p.Dropoff.MobileNumber,
				Short: true,
			},
		},
	}

	pickUp := &Attachment{
		Title: "Delivery Pickup",
		Color: "#bf2b2b",
		Text:  fmt.Sprintf("%s %s %s, %s, %s", p.Pickup.AddressOne, p.Pickup.AddressTwo, p.Pickup.City, p.Pickup.State, p.Pickup.ZipCode),
		Fields: []*Field{
			&Field{
				Title: "Lat",
				Value: strconv.FormatFloat(p.Pickup.Lat, 'E', -1, 32),
				Short: true,
			},
			&Field{
				Title: "Lng",
				Value: strconv.FormatFloat(p.Pickup.Lng, 'E', -1, 32),
				Short: true,
			},
			&Field{
				Title: "Name",
				Value: p.Pickup.DisplayName,
				Short: true,
			},
			&Field{
				Title: "Number",
				Value: p.Pickup.MobileNumber,
				Short: true,
			},
		},
	}

	destinationURL, err := url.Parse(baseMapsURL)
	if err != nil {
		return nil, err
	}
	params := url.Values{}
	params.Add("api", "1")
	params.Add("origin", pickUp.Text)
	params.Add("destination", dest.Text)
	params.Add("travelmode", "bicycling")
	destinationURL.RawQuery = params.Encode()

	route := &Attachment{
		Title: "Delivery Directions",
		Color: "#3cff00",
		Text:  destinationURL.String(),
	}

	return &PostMatesSlack{
		Title: fmt.Sprintf("Received postmates dispatch with ID: %s", p.ID),
		Attachments: []*Attachment{
			pickUp,
			dest,
			route,
		},
	}, nil
}
