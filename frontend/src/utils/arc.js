/**
 * arc.js
 * =============================================================================
 * Utility functions & calculations to support the enhanced arc mode navigation
 */
import * as trans from './translations'
import * as proj from './projections'
import * as pag from './pointandgo'
import { Vector2 } from './vectors'

// Number of samples grab from the trajectory (higher = better resolution
// of curves)
export const ARC_SAMPLE_SIZE = 100
export const Z_MAX = 10
export const WORLD_X_MAX = 8
export const CALIBRATED_X_MAX = 5
export const VEHICLE_WIDTH = 1 // In meters
export const CAMERA_HEIGHT = 1 // In meters

/**
 * Returns an array of points along a curvature
 * @param  {Number} x              x
 * @param  {Number} y              y
 * @param  {Number} z              z
 * @param  {Number} numSample      Number of samples
 * @param  {Number} cameraHeight   Height of camera
 * @param  {Number} rOffset        Offset for r (pass in [-1 * (vehicle width
 * / 2)] here if solving for path of the right side of the vehicle)
 * @return {Array}                Array of [x,y]s in the calibrated space
 */
export function curvatureToCalibratedPoints(
  x,
  y,
  z,
  numSample,
  cameraHeight,
  rOffset = 0
) {
  // Find out where the curvature center exists,
  //    r^2 = (r-x)^2 + z^2 <-> r = (x^2 + z^2) / (2x)
  const r = (x * x + z * z) / (2 * x)
  if (r === 0) {
    // r is zero, no curvature drawn
    return false
  }

  const points = []

  let xp
  let zp
  let calibratedPoint

  // Right side of vertical axis (+x)
  if (r > 0) {
    if (z > Z_MAX) {
      return []
    }
    const thetaMax = z < x ? Math.PI - Math.asin(z / r) : Math.asin(z / r)
    for (let theta = 0; theta <= thetaMax; theta += thetaMax / numSample) {
      if (theta === 0) continue

      xp = r - (r + rOffset) * Math.cos(theta)
      zp = (r + rOffset) * Math.sin(theta)
      calibratedPoint = new Vector2(xp / zp, cameraHeight / zp)
      if (calibratedPoint.y < 0) {
        continue // Skip this point if the calibrated result doesn't make sense
      }
      points.push(new Vector2(calibratedPoint.x, calibratedPoint.y))
    }
  } else {
    // Left side of vertical axis (-x)
    if (z > Z_MAX) {
      return []
    }
    // if r < 0
    const thetaMax = z < -x ? Math.PI - Math.asin(-z / r) : Math.asin(-z / r)
    for (let theta = 0; theta <= thetaMax; theta += thetaMax / numSample) {
      if (theta === 0) continue

      xp = r - (r + rOffset) * Math.cos(theta)
      zp = -(r + rOffset) * Math.sin(theta)
      calibratedPoint = new Vector2(xp / zp, cameraHeight / zp)
      if (calibratedPoint.y < 0) {
        continue // Skip this point if the calibrated result doesn't make sense
      }
      points.push(new Vector2(calibratedPoint.x, calibratedPoint.y))
    }
  }

  return points
}

export function mapCalibratedPointsToPixels(
  calibratedPointArray = [],
  focalLength,
  cameraCenter,
  kVals
) {
  return calibratedPointArray.map(v => {
    return new proj.ProjectCalibratedPointToImage(
      v,
      focalLength,
      cameraCenter,
      kVals
    )
  })
}

export function mapPixelPointsToCanvas(
  pixelPointArray = [],
  resolutionX,
  resolutionY,
  canvasWidth,
  canvasHeight
) {
  return pixelPointArray.map(v => {
    return trans.scaleImageToCanvas(
      v,
      resolutionX,
      resolutionY,
      canvasWidth,
      canvasHeight
    )
  })
}

/**
 * Create curvature
 * @param  {Vector3} point           Point in calibrated camera space
 * @param  {Number} numSample        Number of samples
 * @param  {Number} cameraHeight     Camera height
 * @param  {Number} rOffset          Offset for r (pass in [-1 * (vehicle
 * width / 2)] here if solving for path of the right side of the vehicle)
 * @param  {Object} intrinsicsObject Intrinsics object
 * @param  {Object} canvasDimensions Camera dimensions
 * @return {[type]}                  [description]
 */
export function curvatureCanvasPoints(
  point,
  numSample,
  cameraHeight,
  rOffset,
  intrinsicsObject,
  canvasDimensions
) {
  const calibratedPoints = curvatureToCalibratedPoints(
    point.x,
    point.y,
    point.z,
    numSample,
    cameraHeight,
    rOffset
  )
  const pixelPoints = mapCalibratedPointsToPixels(
    calibratedPoints,
    intrinsicsObject.focalLength,
    intrinsicsObject.cameraCenter,
    intrinsicsObject.kVals
  )
  const canvasPoints = mapPixelPointsToCanvas(
    pixelPoints,
    intrinsicsObject.resolution.x,
    intrinsicsObject.resolution.y,
    canvasDimensions.x,
    canvasDimensions.y
  )

  return canvasPoints
}

/**
 * Draw a set of arcs (relative to a point on the canvas) on the canvas
 * @param  {Object}  ctx              Canvas context
 * @param  {Vector2} canvasPt         Point in canvas space
 * @param  {Object}  intrinsics       Intrinsics object
 * @param  {Object}  canvasDimensions Dimensions of canvas
 * @param  {Number}  cameraHeight     Camera height
 * @param  {Number}  sampleSize       Sample size (higher is better but slower)
 * @param  {Number}  vehicleWidth     Vehicle width
 */
export function drawArcs(
  ctx,
  canvasPt,
  intrinsics,
  canvasDimensions,
  cameraHeight,
  sampleSize,
  vehicleWidth
) {
  const calibratedPt = pag.canvasPointToCalibratedPoint(
    canvasPt,
    intrinsics,
    canvasDimensions
  )
  const cameraPt = pag.calibratedPointToCameraPoint(calibratedPt)

  // Don't draw if...
  if (
    // We're outside of the canvas dimensions
    canvasPt.x > canvasDimensions.x ||
    canvasPt.y > canvasDimensions.y ||
    // The calibrated coordinate is out of bounds
    calibratedPt.x > CALIBRATED_X_MAX ||
    calibratedPt.x < -CALIBRATED_X_MAX ||
    // We're outside of the camera's field of view
    cameraPt.x > WORLD_X_MAX ||
    cameraPt.x < -WORLD_X_MAX
  ) {
    return
  }

  const centerPath = new Path2D()
  const leftPath = new Path2D()
  const rightPath = new Path2D()

  curvatureCanvasPoints(
    cameraPt,
    sampleSize,
    cameraHeight,
    0,
    intrinsics,
    canvasDimensions
  ).forEach(v => {
    centerPath.lineTo(v.x, v.y, 2, 2)
  })
  curvatureCanvasPoints(
    cameraPt,
    sampleSize,
    cameraHeight,
    vehicleWidth / 2,
    intrinsics,
    canvasDimensions
  ).forEach(v => {
    leftPath.lineTo(v.x, v.y, 2, 2)
  })
  curvatureCanvasPoints(
    cameraPt,
    sampleSize,
    cameraHeight,
    -vehicleWidth / 2,
    intrinsics,
    canvasDimensions
  ).forEach(v => {
    rightPath.lineTo(v.x, v.y, 2, 2)
  })

  ctx.lineWidth = 2
  ctx.strokeStyle = 'white'
  ctx.setLineDash([15, 5])
  ctx.stroke(leftPath)
  ctx.stroke(rightPath)
  ctx.setLineDash([])
  ctx.strokeStyle = 'goldenrod'
  ctx.stroke(centerPath)
}

/**
 * Converts a canvas point into a proportional image coordinate
 * @param  {Vector2} canvasPoint       Canvas point (x,y)
 * @param  {Vector2} cameraResolution  Resolution of camera (xMax, yMax)
 * @param  {Object}  canvasDimensions  Dimensions of canvas
 * @return {Vector2}                   Percentage in image space
 */
export function canvasPointToImagePercentage(
  canvasPoint,
  cameraResolution,
  canvasDimensions
) {
  // Canvas -> Image
  const imagePt = trans.scaleCanvasToImage(
    canvasPoint,
    cameraResolution.x,
    cameraResolution.y,
    canvasDimensions.x,
    canvasDimensions.y
  )
  const imageMin = new Vector2(0, 0)
  const imageMax = cameraResolution
  // Image -> Image %
  const percent = trans.getPercentageFromRange(imagePt, imageMin, imageMax)

  return new Vector2(percent.x / 100, percent.y / 100)
}
