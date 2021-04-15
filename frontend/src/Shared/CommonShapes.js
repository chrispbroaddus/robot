/**
 * Common shapes for PropTypes
 */
import PropTypes from 'prop-types'

const Camera = PropTypes.shape({
  id: PropTypes.string.isRequired,
  width: PropTypes.number.isRequired,
  height: PropTypes.number.isRequired,
  role: PropTypes.string.isRequired,
  intrinsics: PropTypes.shape({
    camera_id: PropTypes.string.isRequired,
    kannala_brandt: PropTypes.shape({
      radialDistortionCoefficientI: PropTypes.arrayOf(PropTypes.number)
        .isRequired,
      radialDistortionCoefficientK: PropTypes.arrayOf(PropTypes.number)
        .isRequired,
      radialDistortionCoefficientL: PropTypes.arrayOf(PropTypes.number)
        .isRequired,
      tangentialDistortionCoefficientJ: PropTypes.arrayOf(PropTypes.number)
        .isRequired,
      tangentialDistortionCoefficientM: PropTypes.arrayOf(PropTypes.number)
        .isRequired
    }).isRequired,
    opticalCenterX: PropTypes.number.isRequired,
    opticalCenterY: PropTypes.number.isRequired,
    resolutionX: PropTypes.number.isRequired,
    resolutionY: PropTypes.number.isRequired,
    scaledFocalLengthX: PropTypes.number.isRequired,
    scaledFocalLengthY: PropTypes.number.isRequired,
    skew: PropTypes.number.isRequired
  }),
  extrinsics: PropTypes.shape({
    rodriguesRotation: PropTypes.arrayOf(PropTypes.number).isRequired,
    sourceCoordinateFrame: PropTypes.shape({
      name: PropTypes.string.isRequired,
      // TODO: Fill the below in with more specific proptypes
      validPeriodBegin: PropTypes.object,
      validPeriodEnd: PropTypes.object
    }).isRequired,
    targetCoordinateFrame: PropTypes.shape({
      name: PropTypes.string.isRequired,
      // TODO: Fill the below in with more specific proptypes
      validPeriodBegin: PropTypes.object,
      validPeriodEnd: PropTypes.object
    }).isRequired,
    timeOffsetNanoseconds: PropTypes.number.isRequired,
    translation: PropTypes.arrayOf(PropTypes.number).isRequired
  })
})

const Location = PropTypes.shape({
  lat: PropTypes.number.isRequired,
  lon: PropTypes.number.isRequired,
  alt: PropTypes.number.isRequired
})

const Operator = PropTypes.shape({
  name: PropTypes.string.isRequired
})

const Vehicle = PropTypes.shape({
  id: PropTypes.string.isRequired,
  status: PropTypes.string.isRequired,
  vehicle_id: PropTypes.string.isRequired,
  location: PropTypes.shape({
    location: Location.isRequired,
    timestamp: PropTypes.string.isRequired
  }).isRequired,
  cameras: PropTypes.arrayOf(Camera).isRequired,
  operator: Operator
})

const Command = PropTypes.shape({
  id: PropTypes.string.isRequired,
  type: PropTypes.string.isRequired,
  status: PropTypes.string.isRequired
})

const VehicleKV = (arr, idx, componentName, location, propFullName) => {
  var obj = arr[idx]
  var props = {}
  props[propFullName] = obj

  // Check if `obj` is an Object using `PropTypes.object`
  var isObjectError = PropTypes.object(
    props,
    propFullName,
    componentName,
    location
  )
  if (isObjectError) {
    return isObjectError
  }

  // Check if all of its keys are strings
  var validObjectKeysError = PropTypes.string
  if (validObjectKeysError) {
    return validObjectKeysError
  }

  // Check if all of its values are type of Vehicle
  var validObjectValues = PropTypes.objectOf(Vehicle)
  var validObjectValuesError = validObjectValues(
    props,
    propFullName,
    componentName,
    location
  )
  if (validObjectValuesError) {
    return validObjectValuesError
  }

  return null
}

const Notifications = PropTypes.shape({
  message: PropTypes.string.isRequired,
  timeout: PropTypes.number.isRequired,
  status: PropTypes.string.isRequired
})

const CommonShapes = {
  Camera,
  Vehicle,
  Command,
  Notifications,
  Location,
  Operator,
  VehicleKV
  // Maybe add telemetry data shapes here as well
}

export default CommonShapes
