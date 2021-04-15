/**
 * manifest.js
 * =============================================================================
 * Utility functions for managing the camera's nullable manifest values
 */
import manifest from './manifest.test.json'
import { Vector2, Vector3 } from './vectors'

/**
 * Creates a manifest object, replacing null or invalid values with a default
 * manifest
 * @param  {Object} camera Camera object
 * @return {Object}        Manifest object with two keys: intrinsics,
 *                         extrinsics, & isDefault
 */
export function manifestFactory(camera) {
  let useDefault = false
  let isIntrinsicsValid =
    camera && camera.intrinsics
      ? validateRawIntrinsics(camera.intrinsics)
      : null
  let isExtrinsicsValid =
    camera && camera.extrinsics
      ? validateRawExtrinsics(camera.extrinsics)
      : null

  if (camera === null) {
    console.log('Loaded default manifest')
    useDefault = true
  } else if (!isIntrinsicsValid && !isExtrinsicsValid) {
    console.log(
      'Camera intrinsics AND extrinsics are either null or invalid. Loading default manifest.'
    )
    useDefault = true
  } else if (!isIntrinsicsValid) {
    console.log(
      'Camera intrinsics are either null or invalid. Loading default manifest.'
    )
    useDefault = true
  } else if (!isExtrinsicsValid) {
    console.log(
      'Camera extrinsics are either null or invalid. Loading default manifest.'
    )
    useDefault = true
  }

  if (useDefault === true) {
    return {
      cameraId: null,
      intrinsics: getDefaultIntrinsics(),
      extrinsics: getDefaultExtrinsics(),
      isDefault: true
    }
  } else {
    console.log('Loaded manifest from vehicle')
    return {
      cameraId: camera.id,
      intrinsics: mapIntrinsics(camera.intrinsics),
      extrinsics: mapExtrinsics(camera.extrinsics),
      isDefault: false
    }
  }
}

/**
 * Returns the default intrinsics read from the sample manifest
 * @return {Object} Intrinsics object
 */
export const getDefaultIntrinsics = () => {
  const intrinsics = manifest.cameraIntrinsicCalibration.filter(
    v => v.cameraUnderCalibration.name === 'camera0'
  )[0]
  console.assert(
    intrinsics.skew === 0,
    "Camera's skew is NOT 0. The project/unproject functions are not designed for skews other than 0 and will need to be modified."
  )
  return {
    resolution: new Vector2(intrinsics.resolutionX, intrinsics.resolutionY),
    focalLength: new Vector2(
      intrinsics.scaledFocalLengthX,
      intrinsics.scaledFocalLengthY
    ),
    cameraCenter: new Vector2(
      intrinsics.opticalCenterX,
      intrinsics.opticalCenterY
    ),
    kVals: [...intrinsics.kannalaBrandt.radialDistortionCoefficientK],
    skew: intrinsics.skew
  }
}

/**
 * Returns the default extrinsics read from the sample manifest
 * @return {Object} Extrinsics object
 */
export const getDefaultExtrinsics = () => {
  const extrinsics = manifest.deviceToDeviceCoordinateTransformation[0]
  return {
    rodriguesRotation: new Vector3(
      extrinsics.rodriguesRotationX,
      extrinsics.rodriguesRotationY,
      extrinsics.rodriguesRotationZ
    ),
    sourceCoordinateFrame: {
      name: extrinsics.sourceCoordinateFrame.device.name
    },
    targetCoordinateFrame: {
      name: extrinsics.targetCoordinateFrame.device.name
    },
    translation: new Vector3(
      extrinsics.translationX,
      extrinsics.translationY,
      extrinsics.translationZ
    )
  }
}

/**
 * Validates the intrinsics object coming directly from the vehicle manifest
 * @param  {Object} intrinsics Intrinsics object
 * @return {Boolean}           True if valid, false if not
 */
export function validateRawIntrinsics(intrinsics) {
  try {
    const result =
      !!intrinsics === true &&
      (!!intrinsics.resolutionX === true || intrinsics.resolutionX === 0) &&
      (!!intrinsics.resolutionY === true || intrinsics.resolutionY === 0) &&
      (!!intrinsics.scaledFocalLengthX === true ||
        intrinsics.scaledFocalLengthX === 0) &&
      (!!intrinsics.scaledFocalLengthY === true ||
        intrinsics.scaledFocalLengthY === 0) &&
      (!!intrinsics.opticalCenterX === true ||
        intrinsics.opticalCenterX === 0) &&
      (!!intrinsics.opticalCenterY === true ||
        intrinsics.opticalCenterY === 0) &&
      (!!intrinsics.kannala_brandt === true ||
        intrinsics.kannala_brandt === 0) &&
      Array.isArray(intrinsics.kannala_brandt.radialDistortionCoefficientK) &&
      (!!intrinsics.skew === true || intrinsics.skew === 0)
    return result
  } catch (err) {
    return false
  }
}

/**
 * Validates the extrinsics object coming directly from the vehicle manifest
 * @param  {Object} extrinsics Extrinsics object
 * @return {Boolean}           True if valid, false if not
 */
export function validateRawExtrinsics(extrinsics) {
  try {
    const result =
      !!extrinsics === true &&
      Array.isArray(extrinsics.rodriguesRotation) &&
      (!!extrinsics.sourceCoordinateFrame === true ||
        extrinsics.sourceCoordinateFrame === 0) &&
      (!!extrinsics.targetCoordinateFrame === true ||
        extrinsics.targetCoordinateFrame === 0) &&
      Array.isArray(extrinsics.translation)
    return result
  } catch (err) {
    return false
  }
}

/**
 * Maps the intrinsics from the camera's manifest to the format used in the
 * frontend
 * @param  {Object} intrinsics Intrinsics object from the camera
 * @return {Object}            Intrinsics object for the UI
 */
export function mapIntrinsics(intrinsics) {
  try {
    console.assert(
      intrinsics.skew === 0,
      "Camera's skew is NOT 0. The project/unproject functions are not designed for skews other than 0 and will need to be modified."
    )
    return {
      resolution: new Vector2(intrinsics.resolutionX, intrinsics.resolutionY),
      focalLength: new Vector2(
        intrinsics.scaledFocalLengthX,
        intrinsics.scaledFocalLengthY
      ),
      cameraCenter: new Vector2(
        intrinsics.opticalCenterX,
        intrinsics.opticalCenterY
      ),
      kVals: [...intrinsics.kannala_brandt.radialDistortionCoefficientK],
      skew: intrinsics.skew
    }
  } catch (err) {
    throw new Error('Error occured while mapping intrinsics', err)
  }
}
/**
 * Maps the extrinsics from the camera's manifest to the format used in the
 * frontend
 * @param  {Object} extrinsics Extrinsics object from the camera
 * @return {Object}            Extrinsics object for the UI
 */
export function mapExtrinsics(extrinsics) {
  try {
    return {
      rodriguesRotation: new Vector3(
        extrinsics.rodriguesRotationX,
        extrinsics.rodriguesRotationY,
        extrinsics.rodriguesRotationZ
      ),
      sourceCoordinateFrame: {
        name: extrinsics.sourceCoordinateFrame.name
      },
      targetCoordinateFrame: {
        name: extrinsics.targetCoordinateFrame.name
      },
      translation: new Vector3(
        extrinsics.translationX,
        extrinsics.translationY,
        extrinsics.translationZ
      )
    }
  } catch (err) {
    throw new Error('Error occured while mapping extrinsics', err)
  }
}
