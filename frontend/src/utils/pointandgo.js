/**
 * pointandgo.js
 * =============================================================================
 * Utility functions & calculations to support the enhanced point and go
 * navigation
 */
import * as trans from './translations'
import * as proj from './projections'
import { Vector2, Vector3 } from './vectors'

export const SAMPLE_SIZE = 20
export const WORLD_X_MAX = 8
export const CALIBRATED_X_MAX = 5

/**
 * Converts a canvas coordinate to a calibrated coordinate
 * @param  {Vector2}  canvasPt         Canvas coordinate
 * @param  {Object}   intrinsicsObject Camera Intrinsics object
 * @param  {Vector2}  canvasDimensions Canvas dimensions
 * @return {Vector2}                    Camera coordinate
 */
export function canvasPointToCalibratedPoint(
  canvasPt,
  intrinsicsObject,
  canvasDimensions
) {
  const { resolution, focalLength, cameraCenter, kVals } = intrinsicsObject
  // Canvas pixel point to Image pixel point
  const imagePt = trans.scaleCanvasToImage(
    canvasPt,
    resolution.x,
    resolution.y,
    canvasDimensions.x,
    canvasDimensions.y
  )
  // Image pixel point to Calibrated point
  const calibratedPt = proj.UnprojectImageToCalibratedPoint(
    imagePt,
    focalLength,
    cameraCenter,
    kVals
  )

  return new Vector2(calibratedPt.x, calibratedPt.y)
}

/**
 * Converts a calibrated point to a camera point
 * @param  {Vector2} calibratedPoint Calibrated point
 * @param  {Number}  cameraHeight    Height of camera, defaults to 1
 * @return {Vector3}                 Camera point
 */
export function calibratedPointToCameraPoint(
  calibratedPoint,
  cameraHeight = 1
) {
  //  Find the intersection between the ray penetrating calibratedClickedPoint
  //  and the y=cameraHeight plane.
  if (calibratedPoint.y <= 0) {
    return new Vector2(null, null)
  }

  const z = cameraHeight / calibratedPoint.y
  const x = calibratedPoint.x * z
  const y = calibratedPoint.y * z

  return new Vector3(x, y, z)
}

/**
 * Project a calibrated point to the canvas space
 * @param  {Vector2} calibratedPoint  Calibrated point
 * @param  {Object}  intrinsicsObject Intrinsics object
 * @param  {Vector2} canvasDimensions Vector2 of canvas dimensions
 * @return {Vector2}                  Vector2 of the canvas point
 */
export function calibratedPointToCanvasPoint(
  calibratedPoint,
  intrinsicsObject,
  canvasDimensions
) {
  const { focalLength, cameraCenter, kVals, resolution } = intrinsicsObject

  const imagePt = proj.ProjectCalibratedPointToImage(
    calibratedPoint,
    focalLength,
    cameraCenter,
    kVals
  )

  return trans.scaleImageToCanvas(
    imagePt,
    resolution.x,
    resolution.y,
    canvasDimensions.x,
    canvasDimensions.y
  )
}

/**
 * Converts an array of calibrated points into an array of canvas points
 * @param  {Array}   calibratedPoints Array of calibrated points
 * @param  {Number}  cameraHeight     Camera height
 * @param  {Object}  intrinsicsObject Intrinsics
 * @param  {Vector2} canvasDimensions Canvas dimensions
 * @return {Array}                    Array of [x,y] points
 */
export function cameraSpaceObjectToCanvasPtArray(
  calibratedPoints,
  cameraHeight,
  intrinsicsObject,
  canvasDimensions
) {
  const points = []
  for (let i = 0; i < calibratedPoints.length; i++) {
    let { x, y } = calibratedPointToCanvasPoint(
      convert3DTo2D(calibratedPoints[i], cameraHeight),
      intrinsicsObject,
      canvasDimensions
    )
    points.push([x, y])
  }
  return points
}

/**
 * Converts a 3D coordinate to 2D
 * @param  {Vector2} point Vector2 of the point
 * @param  {Height}  h     Height
 * @return {Vector2}       Vector2
 */
export function convert3DTo2D(point, h) {
  return {
    x: point.x / point.y,
    y: h / point.y
  }
}

/**
 * Creates a path and draws an array of points onto the canvas; can optionally
 * fill
 * @param  {Object}  ctx        Canvas context
 * @param  {Array}  points      Array of [x,y] elements
 * @param  {String}  color      Color of line
 * @param  {Boolean} shouldFill Fill shape flag
 */
export function drawCanvasPtArray(
  ctx,
  points,
  color = 'blue',
  shouldFill = false
) {
  const path = new Path2D()
  ctx.lineWidth = 2
  ctx.strokeStyle = color
  ctx.fillStyle = color
  for (let p = 0; p < points.length; p++) {
    path.lineTo(points[p][0], points[p][1], 2, 2)
  }
  path.closePath()
  if (shouldFill) {
    ctx.fill(path)
  } else {
    ctx.stroke(path)
  }
}

/**
 * Factory for a box camera object
 * @param  {Vector2} calibratedPt Calibrated point
 * @param  {Number} width         Width of box
 * @param  {Number} height        Height of box
 * @param  {Number} theta         Angle of rotation (in radians)
 * @param  {Number} cameraHeight  Camera height
 * @return {Array}                Returns an array of calibrated points
 */
export function boxCameraObjectFactory(
  calibratedPt,
  width,
  height,
  theta,
  cameraHeight = 1,
  sampleSize = 1
) {
  // Center of our box
  const xc = cameraHeight / calibratedPt.y * calibratedPt.x
  const zc = cameraHeight / calibratedPt.y

  // Don't draw if we're in these constraints
  if (xc < -10 || xc > 10 || zc > 10 || zc < 0.75) {
    return []
  }

  const points = []
  // Top left -> Top right
  for (let i = -width / 2; i <= width / 2; i += width / sampleSize) {
    points.push(
      new Vector2(
        xc + (Math.cos(theta) * i - Math.sin(theta) * height / 2),
        zc + (Math.sin(theta) * i + Math.cos(theta) * height / 2)
      )
    )
  }
  // Top right -> bottom right
  for (let i = height / 2; i >= -height / 2; i -= height / sampleSize) {
    points.push(
      new Vector2(
        xc + (Math.cos(theta) * width / 2 - Math.sin(theta) * i),
        zc + (Math.sin(theta) * width / 2 + Math.cos(theta) * i)
      )
    )
  }
  // Bottom right -> bottom left
  for (let i = width / 2; i >= -width / 2; i -= width / sampleSize) {
    points.push(
      new Vector2(
        xc + (Math.cos(theta) * i - Math.sin(theta) * -height / 2),
        zc + (Math.sin(theta) * i + Math.cos(theta) * -height / 2)
      )
    )
  }
  // Bottom left -> top left
  for (let i = -height / 2; i <= height / 2; i += height / sampleSize) {
    points.push(
      new Vector2(
        xc + (Math.cos(theta) * -width / 2 - Math.sin(theta) * i),
        zc + (Math.sin(theta) * -width / 2 + Math.cos(theta) * i)
      )
    )
  }

  return points
}

/**
 * Factory for a triangle camera object
 * @param  {Vector2} calibratedPt Calibrated point
 * @param  {Number} width         Width of box
 * @param  {Number} height        Height of box
 * @param  {Number} theta         Angle of rotation (in radians)
 * @param  {Number} cameraHeight  Camera height
 * @return {Array}                Array of calibrated points
 */
export function triangleCameraObjectFactory(
  calibratedPt,
  width,
  height,
  theta,
  cameraHeight = 1
) {
  // Center of our box
  const xc = cameraHeight / calibratedPt.y * calibratedPt.x
  const zc = cameraHeight / calibratedPt.y

  // Don't draw if we're in these constraints
  if (xc < -10 || xc > 10 || zc > 10 || zc < 0.75) {
    return []
  }

  return [
    // Left
    new Vector2(
      xc +
        (Math.cos(theta) * (-width / 6) - Math.sin(theta) * (5 * height / 8)),
      zc + (Math.sin(theta) * (-width / 6) + Math.cos(theta) * (5 * height / 8))
    ),
    // Right
    new Vector2(
      xc + (Math.cos(theta) * (width / 6) - Math.sin(theta) * (5 * height / 8)),
      zc + (Math.sin(theta) * (width / 6) + Math.cos(theta) * (5 * height / 8))
    ),
    // Top
    new Vector2(
      xc +
        (Math.cos(theta) * (0 / 2) -
          Math.sin(theta) * (1.1 * (6 * height / 8))),
      zc +
        (Math.sin(theta) * (0 / 2) + Math.cos(theta) * (1.1 * (6 * height / 8)))
    )
  ]
}

/**
 * Draws a distorted rectangle from the canvas space to the canvas space
 * @param  {Object}  ctx              Canvas context
 * @param  {Vector2} canvasPt         Canvas point (x,y)
 * @param  {Number}  theta            Rotation in radians
 * @param  {Object}  intrinsics       Intrinsics object
 * @param  {Vector2} canvasDimensions Canvas dimensions
 * @param  {Boolean} drawDirection    Flag to draw the heading (via triangle)
 */
export function drawDistortedRect(
  ctx,
  canvasPt,
  theta,
  intrinsics,
  canvasDimensions,
  drawDirection = false
) {
  const calibratedPt = canvasPointToCalibratedPoint(
    canvasPt,
    intrinsics,
    canvasDimensions
  )
  const cameraPt = calibratedPointToCameraPoint(calibratedPt)

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

  // Dimensions of robot
  const width = 1 // In meters
  const height = 1 // In meters
  const cameraHeight = 1

  // Create our camera objects
  const box = boxCameraObjectFactory(
    calibratedPt,
    width,
    height,
    theta,
    cameraHeight,
    SAMPLE_SIZE // Sample size
  )
  const triangle = triangleCameraObjectFactory(
    calibratedPt,
    width,
    height,
    theta,
    cameraHeight
  )

  // Convert the camera objects into an array of points and draw
  drawCanvasPtArray(
    ctx,
    cameraSpaceObjectToCanvasPtArray(
      box,
      cameraHeight,
      intrinsics,
      canvasDimensions
    ),
    '#FE7867'
  )
  if (drawDirection === true) {
    drawCanvasPtArray(
      ctx,
      cameraSpaceObjectToCanvasPtArray(
        triangle,
        cameraHeight,
        intrinsics,
        canvasDimensions
      ),
      '#FE7867',
      true
    )
  }
}

/**
 * Draws a distorted rectangle from the canvas space to the canvas space
 * @param  {Object}  ctx              Canvas context
 * @param  {Vector2} calibratedPt     Calibrated point
 * @param  {Number}  theta            Rotation in radians
 * @param  {Object}  intrinsics       Intrinsics object
 * @param  {Vector2} canvasDimensions Canvas dimensions
 * @param  {Boolean} drawDirection    Flag to draw the heading (via triangle)
 */
export function createDistortedRectInCalibratedSpace(
  ctx,
  calibratedPt,
  theta,
  intrinsics,
  canvasDimensions,
  drawDirection = false
) {
  // Dimensions of robot
  const width = 1 // In meters
  const height = 1 // In meters
  const cameraHeight = 1

  // Create our camera objects
  const box = boxCameraObjectFactory(
    calibratedPt,
    width,
    height,
    theta,
    cameraHeight
  )
  const triangle = triangleCameraObjectFactory(
    calibratedPt,
    width,
    height,
    theta,
    cameraHeight
  )

  // Convert the camera objects into an array of points and draw
  drawCanvasPtArray(
    ctx,
    cameraSpaceObjectToCanvasPtArray(
      box,
      cameraHeight,
      intrinsics,
      canvasDimensions
    ),
    '#FE7867'
  )
  if (drawDirection === true) {
    drawCanvasPtArray(
      ctx,
      cameraSpaceObjectToCanvasPtArray(
        triangle,
        cameraHeight,
        intrinsics,
        canvasDimensions
      ),
      '#FE7867',
      true
    )
  }
}
