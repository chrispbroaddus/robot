/**
 * translations.js
 * =============================================================================
 * Utility functions & calculations to support the enhanced arc mode navigation
 */
import { Vector2 } from './vectors'

/**
 * Turns a image pixel coordinate to a scaled canvas coordinates
 * @param  {Object} imagePoint     Vector2 image coord
 * @param  {Number} imageWidth     Width of image in pixels
 * @param  {Number} imageHeight    Height of image in pixels
 * @param  {Number} canvasWidth    Width of canvas in pixels
 * @param  {Number} canvasHeight   Height of canvas in pixels
 * @return {Object}                Vector2 canvas coord
 */
export const scaleImageToCanvas = (
  imagePoint,
  imageWidth,
  imageHeight,
  canvasWidth,
  canvasHeight
) => {
  const imageMin = new Vector2(0, 0)
  const imageMax = new Vector2(imageWidth, imageHeight)
  const canvasMin = new Vector2(0, 0)
  const canvasMax = new Vector2(canvasWidth, canvasHeight)

  return scaleAtoBVector(imagePoint, imageMin, imageMax, canvasMin, canvasMax)
}

/**
 * Turns a canvas coordinate to a scaled image coordinate
 * @param  {Object} canvasPoint     Vector2 canvas coord
 * @param  {Number} imageWidth     Width of image in pixels
 * @param  {Number} imageHeight    Height of image in pixels
 * @param  {Number} canvasWidth    Width of canvas in pixels
 * @param  {Number} canvasHeight   Height of canvas in pixels
 * @return {Object}                 Vector2 image coord
 */
export const scaleCanvasToImage = (
  canvasPoint,
  imageWidth,
  imageHeight,
  canvasWidth,
  canvasHeight
) => {
  const imageMin = new Vector2(0, 0)
  const imageMax = new Vector2(imageWidth, imageHeight)
  const canvasMin = new Vector2(0, 0)
  const canvasMax = new Vector2(canvasWidth, canvasHeight)

  return scaleAtoBVector(canvasPoint, canvasMin, canvasMax, imageMin, imageMax)
}

/**
 * Returns a vector in Range B that's proportional to the vector in Range A
 * @param  {Object} point Vector2 Point from range A
 * @param  {Object} aMin  Vector2 (x,y) minima from A
 * @param  {Object} aMax  Vector2 (x,y) maxima from A
 * @param  {Object} bMin  Vector2 (x,y) minima from B
 * @param  {Object} bMax  Vector2 (x,y) maxima from B
 * @return {Object}       Vector2 Point translated to range B
 */
export const scaleAtoBVector = (point, aMin, aMax, bMin, bMax) => {
  // Find the percent values of the pixel space
  const percent = getPercentageFromRange(point, aMin, aMax)

  return new Vector2(
    percent.x * (bMax.x - bMin.x) / 100 + bMin.x,
    percent.y * (bMax.y - bMin.y) / 100 + bMin.y
  )
}

/**
 * Returns a value in Range B that's proportional to the value in Range A
 * (single dimension)
 * @param  {Number} value Value in A
 * @param  {Number} aMin  A minimum
 * @param  {Number} aMax  A maximum
 * @param  {Number} bMin  B minimum
 * @param  {Number} bMax  B maximum
 * @return {Number}       Value translated to range B
 */
export const scaleAtoB = (value, aMin, aMax, bMin, bMax) => {
  // Find the percent values of the pixel space
  const percent = getPercentageFromRange(
    { x: value, y: 0 },
    { x: aMin, y: 0 },
    { x: aMax, y: 0 }
  )

  return percent.x * (bMax - bMin) / 100 + bMin
}

/**
 * Returns the percentage value of the point in the range provided
 * @param  {Object} point Vector2 of the point
 * @param  {Object} min   Vector2 of minima
 * @param  {Object} max   Vector2 of maxima
 * @return {Object}       Vector2
 */
export const getPercentageFromRange = (point, min, max) => {
  return new Vector2(
    (point.x - min.x) * 100 / (max.x - min.x),
    (point.y - min.y) * 100 / (max.y - min.y)
  )
}
