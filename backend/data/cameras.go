package data

import (
	"sort"
	"time"

	calibration "github.com/zippyai/zippy/packages/calibration/proto"
	core "github.com/zippyai/zippy/packages/core/proto"
)

// Camera is the information related to a vehicles cameras including past samples
type Camera struct {
	ID string `json:"id"`
	// Roles are defined in camera_id.pb.go when the enum is located
	Role       string        `json:"role"`
	Serial     uint64        `json:"serial_number"`
	Width      int           `json:"width"`
	Height     int           `json:"height"`
	Frames     CameraSamples `json:"camera_frames"`
	Intrinsics *Intrinsics   `json:"intrinsics"`
	Extrinsics *Extrinsics   `json:"extrinsics"`
}

// IntrinsicsToStorage is used to convert a proto intrinsics struct to one we put in redis
func IntrinsicsToStorage(intrinsics *calibration.CameraIntrinsicCalibration) *Intrinsics {
	var pinhole *Pinhole
	var kannalaBrandt *KannalaBrandt

	if intrinsics.GetCameraUnderCalibration() == nil {
		return nil
	}

	if intrinsics.GetPinhole() != nil {
		pinhole = &Pinhole{
			Dummy: intrinsics.GetPinhole().Dummy,
		}
	}

	if kb := intrinsics.GetKannalaBrandt(); kb != nil {
		kannalaBrandt = &KannalaBrandt{
			RadialDistortionCoefficientI: [4]float64{
				kb.RadialDistortionCoefficientI1,
				kb.RadialDistortionCoefficientI2,
				kb.RadialDistortionCoefficientI3,
				kb.RadialDistortionCoefficientI4,
			},
			RadialDistortionCoefficientK: kb.RadialDistortionCoefficientK,
			RadialDistortionCoefficientL: [3]float64{
				kb.RadialDistortionCoefficientL1,
				kb.RadialDistortionCoefficientL2,
				kb.RadialDistortionCoefficientL3,
			},
			TangentialDistortionCoefficientM: [3]float64{
				kb.TangentialDistortionCoefficientM1,
				kb.TangentialDistortionCoefficientM2,
				kb.TangentialDistortionCoefficientM3,
			},
			TangentialDistortionCoefficientJ: [4]float64{
				kb.TangentialDistortionCoefficientJ1,
				kb.TangentialDistortionCoefficientJ2,
				kb.TangentialDistortionCoefficientJ3,
				kb.TangentialDistortionCoefficientJ4,
			},
		}
	}

	return &Intrinsics{
		CameraID:           intrinsics.GetCameraUnderCalibration().Name,
		OpticalCenterX:     intrinsics.OpticalCenterX,
		OpticalCenterY:     intrinsics.OpticalCenterY,
		ResolutionX:        intrinsics.ResolutionX,
		ResolutionY:        intrinsics.ResolutionY,
		ScaledFocalLengthX: intrinsics.ScaledFocalLengthX,
		ScaledFocalLengthY: intrinsics.ScaledFocalLengthY,
		Skew:               intrinsics.Skew,
		KannalaBrandt:      kannalaBrandt,
		Pinhole:            pinhole,
	}
}

// Intrinsics is used for all camera intrinsics coming up from the vehicle manifest
// either kannalbrandt or pinhole should be present but not both
type Intrinsics struct {
	CameraID           string         `json:"camera_id"`
	ScaledFocalLengthX float64        `json:"scaledFocalLengthX"`
	Skew               float64        `json:"skew"`
	OpticalCenterX     float64        `json:"opticalCenterX"`
	ScaledFocalLengthY float64        `json:"scaledFocalLengthY"`
	OpticalCenterY     float64        `json:"opticalCenterY"`
	ResolutionX        uint32         `json:"resolutionX"`
	ResolutionY        uint32         `json:"resolutionY"`
	KannalaBrandt      *KannalaBrandt `json:"kannala_brandt,omitempty"`
	Pinhole            *Pinhole       `json:"pinhole,omitempty"`
}

// KannalaBrandt is a dirrect copy of calibration.KnnalaBrandt
type KannalaBrandt struct {
	// / k1...km coefficients in Eq. 6
	RadialDistortionCoefficientK []float64 `json:"radialDistortionCoefficientK,omitempty"`
	// / Eq. 8; set this to zero if you want a pure radial-only model
	RadialDistortionCoefficientL [3]float64 `json:"radialDistortionCoefficientL,omitempty"`
	// / Eq. 8; set this to zero if you want a pure radial-only model
	RadialDistortionCoefficientI [4]float64 `json:"radialDistortionCoefficientI,omitempty"`
	// / Eq. 9; set this to zero if you want a pure radial-only model
	TangentialDistortionCoefficientM [3]float64 `json:"tangentialDistortionCoefficientM,omitempty"`
	// / Eq. 9; set this to zero if you want a pure radial-only model
	TangentialDistortionCoefficientJ [4]float64 `json:"tangentialDistortionCoefficientJ,omitempty"`
}

// Pinhole is a dirrect copy of calibration.Pinhole
type Pinhole struct {
	// / Send with zero length sequence of dummy
	Dummy []float64 `json:"dummy,omitempty"`
}

// ExtrinsicsToStorage is used to convert a proto extrinsics struct to one we put in redis
func ExtrinsicsToStorage(extrinsics *calibration.CoordinateTransformation) *Extrinsics {
	var source, target *CoordinateFrame
	var cameraID string

	if extrinsics.SourceCoordinateFrame != nil {
		if extrinsics.SourceCoordinateFrame.GetDevice() != nil {
			cameraID = extrinsics.SourceCoordinateFrame.GetDevice().Name
		}

		source = &CoordinateFrame{
			ValidPeriodBegin: extrinsics.SourceCoordinateFrame.ValidPeriodBegin,
			ValidPeriodEnd:   extrinsics.SourceCoordinateFrame.ValidPeriodEnd,
			CameraName:       cameraID,
		}
	}

	if extrinsics.TargetCoordinateFrame != nil {
		if extrinsics.TargetCoordinateFrame.GetDevice() != nil {
			cameraID = extrinsics.TargetCoordinateFrame.GetDevice().Name
		}

		target = &CoordinateFrame{
			ValidPeriodBegin: extrinsics.TargetCoordinateFrame.ValidPeriodBegin,
			ValidPeriodEnd:   extrinsics.TargetCoordinateFrame.ValidPeriodEnd,
			CameraName:       cameraID,
		}
	}

	return &Extrinsics{
		SourceCoordinateFrame: source,
		TargetCoordinateFrame: target,
		RodriguesRotation: [3]float64{
			extrinsics.RodriguesRotationX,
			extrinsics.RodriguesRotationY,
			extrinsics.RodriguesRotationZ,
		},
		Translation: [3]float64{
			extrinsics.TranslationX,
			extrinsics.TranslationY,
			extrinsics.TranslationZ,
		},
		TimeOffsetNanoseconds: extrinsics.TimeOffsetNanoseconds,
	}
}

// Extrinsics is used for all camera extrinsics coming up from the vehicle manifest
type Extrinsics struct {
	SourceCoordinateFrame *CoordinateFrame `json:"sourceCoordinateFrame,omitempty"`
	TargetCoordinateFrame *CoordinateFrame `json:"targetCoordinateFrame,omitempty"`
	// / Rotation from the source to the target coordinate frame, represented using so(3).
	// /
	// / The unit vector [rx, ry, rz] / norm([rx, ry, rz]) is the axis in the source frame
	// / about which we will rotate, the magnitude of this vector is the amount by which
	// / we will rotate.
	// /
	// / X component of rotation vector
	RodriguesRotation [3]float64 `json:"rodriguesRotation,omitempty"`
	// / X component of the position of the source coordinate frame in the target coordinate
	// / frame.
	Translation [3]float64 `json:"translation,omitempty"`
	// / Time offset from the clock in the source system to the clock in the target system. When
	// / this is positive, the source system is lagging behind the target system clock.
	TimeOffsetNanoseconds int64 `json:"timeOffsetNanoseconds"`
}

// CoordinateFrame used to identify a possibly mving coordinate frame
type CoordinateFrame struct {
	ValidPeriodBegin *core.SystemTimestamp `json:"validPeriodBegin,omitempty"`
	ValidPeriodEnd   *core.SystemTimestamp `json:"validPeriodEnd,omitempty"`
	CameraName       string                `json:"name"`
}

// CameraSamples is a collection of CameraSamples used for sorting by timeStamp
type CameraSamples []*CameraSample

// Len implementation for the sort interface
func (c CameraSamples) Len() int { return len(c) }

// Less implementation of a comparison for the sort interface
func (c CameraSamples) Less(i, j int) bool {
	return c[i].Timestamp.Before(c[i].Timestamp)
}

// Swap implementation of swapping compaired values for the sort interface
func (c CameraSamples) Swap(i, j int) { c[i], c[j] = c[j], c[i] }

// SamplesSince returns a subset of samples that were taken after the time stamp provided
func (c CameraSamples) SamplesSince(since time.Time) CameraSamples {
	// ensure samples are sorted
	sort.Sort(c)

	var lastIndex int
	for i, sample := range c {
		// once we've found the first time a sample was taken before the timestamp set the right index
		if since.Before(sample.Timestamp) {
			lastIndex = i
			break
		}
	}

	// only return from 0 to the value right before the lastIndex
	return c[:lastIndex]
}

// CameraSample information on a frame that was captured form the owning camera
type CameraSample struct {
	CameraID  string    `json:"camera_id"`
	Timestamp time.Time `json:"timestamp"`
	Width     int       `json:"image_width"`
	Height    int       `json:"image_height"`
	Content   []byte    `json:"-"`
	URL       string    `json:"hosted_url"`
}
