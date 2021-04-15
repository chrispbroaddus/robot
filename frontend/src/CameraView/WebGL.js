import React from 'react'
import PropTypes from 'prop-types'
import styled from 'styled-components'
import { patchEmptyFields } from '../utils'
import * as proj from '../utils/projections'
import * as trans from '../utils/translations'
import { Vector2 } from '../utils/vectors'
import { COLORS } from '../utils/colors'
import { BBOX_CATEGORY } from '../utils/boundingBox'

const Container = styled.div`
  position: absolute;
  top: ${props => (props.top ? `${props.top}px` : 0)};
  left: ${props => (props.left ? `${props.left}px` : 0)};
  width: ${props => (props.width ? `${props.width}px` : '100%')};
  height: ${props => (props.height ? `${props.height}px` : '100%')};
  display: ${props => (props.visible ? 'block' : 'none')};
  z-index: 450;
  user-select: none;

  & > canvas {
    position: absolute;
    top: 0;
    left: 0;
  }

  & > canvas[name='bounding-boxes'] {
    z-index: 101;
  }
  & > canvas[name='3d-bounding-boxes'] {
    z-index: 102;
  }
  & > canvas[name='distortion-grid'] {
    z-index: 100;
  }
  & > canvas[name='ground-plane'] {
    z-index: 90;
  }
`

class WebGL extends React.Component {
  static propTypes = {
    top: PropTypes.number,
    left: PropTypes.number,
    width: PropTypes.number,
    height: PropTypes.number,
    visible: PropTypes.bool,
    manifest: PropTypes.object.isRequired,
    displayDistortionGrid: PropTypes.bool,
    displayGroundPlane: PropTypes.bool,
    displayBoundingBoxes: PropTypes.bool,
    display3dBoundingBoxes: PropTypes.bool,
    displayConvexHulls: PropTypes.bool,
    boundingBoxes: PropTypes.arrayOf(PropTypes.object),
    boundingBoxes3d: PropTypes.arrayOf(PropTypes.object),
    convexHulls: PropTypes.arrayOf(PropTypes.object)
  }

  static defaultProps = {
    top: 0,
    left: 0,
    width: null,
    height: null,
    visible: true,
    displayDistortionGrid: true,
    displayGroundPlane: false,
    displayBoundingBoxes: false,
    display3dBoundingBoxes: false,
    displayConvexHulls: false,
    boundingBoxes: [],
    boundingBoxes3d: [],
    convexHulls: []
  }

  constructor(props) {
    super(props)

    this.canvasGndPlane = null
    this.canvasDistGrid = null
    this.gl = null
    this.canvasWidth = window.innerWidth
    this.canvasHeight = window.innerHeight

    this.colorIndex = 0
  }

  componentDidMount() {
    // Redraw the canvas when we re-render (which is triggered when the video is
    // resized)
    this.readjustCanvasSize()
    this.drawCanvas()
    this.drawBoundingBoxes()
    this.drawBoundingBoxes3d()
    this.drawConvexHulls()
  }

  componentDidUpdate() {
    // Redraw the canvas when we re-render (which is triggered when the video is
    // resized)
    this.readjustCanvasSize()
    this.drawCanvas()
    this.drawBoundingBoxes()
    this.drawBoundingBoxes3d()
    this.drawConvexHulls()
  }

  drawCanvas = () => {
    if (!this.canvasGndPlane || !this.canvasDistGrid) return
    const ctxGP = this.canvasGndPlane.getContext('2d')
    const ctxDG = this.canvasDistGrid.getContext('2d')
    // Disable anti-aliasing
    ctxGP.imageSmoothingEnabled = false
    ctxDG.imageSmoothingEnabled = false

    // Ground plane
    this.drawGrid(ctxGP, this.calibratedPtTranslator, 0.25, 0, 10, -8, 8)

    // Distortion grid
    this.drawPointBox(
      ctxDG,
      this.calibratedPtTranslator,
      0.25,
      -10,
      10,
      -10,
      10
    )
  }

  drawScaledPixel = (ctx, vector, color = 'white') => {
    const { resolution } = this.state

    ctx.lineWidth = 1
    ctx.fillStyle = color
    ctx.beginPath()
    // console.log(`input (${vector.x}, ${vector.y})`)
    const scaledVector = trans.scaleImageToCanvas(
      vector,
      resolution.x,
      resolution.y,
      this.canvasWidth,
      this.canvasHeight
    )
    // console.log(`drawing pixel at (${scaledVector.x}, ${scaledVector.y})`)
    ctx.fillRect(scaledVector.x, scaledVector.y, 2, 2)
    // ctx.arc(scaledVector.x, scaledVector.y, 1, 0, 2 * Math.PI)
    ctx.closePath()
  }

  drawGrid = (ctx, translator, stepSize, zMin, zMax, xMin, xMax) => {
    const y = 1
    let coord
    let calibratedPt
    const baseRgb = '151, 151, 151'
    ctx.lineWidth = 1

    // VERTICAL GRID LINES & POINTS @ INTERSECTIONS
    for (let x = xMin; x <= xMax; x += stepSize) {
      ctx.beginPath()
      let firstZ
      let lastZ
      let alpha

      for (let z = zMin; z <= zMax; z += stepSize) {
        calibratedPt = new Vector2(x / z, y / z)
        // Passing in "true" as the third arg will validate the camera
        // coordinate by projecting & unprojecting and checking if it's within a
        // pre-defined tolerance. Invaild points will return null.
        coord = translator(calibratedPt.x, calibratedPt.y, true)
        // Only draw if the point is valid
        if (coord.x != null && coord.y !== null) {
          // Adjust the alpha of the point to be drawn depending on the depth (z)
          alpha = Math.max(((zMax - z - zMin) / (zMax - zMin)).toFixed(2), 0.2)
          ctx.fillStyle = `rgba(${baseRgb}, ${alpha})`
          // console.log(`at z ${z}, @ ${alpha}`)
          // ctx.fillRect(coord.x, coord.y, 2, 2)
          ctx.lineTo(coord.x, coord.y)
        }

        // Store the first and last coordinate to use in our gradient for
        // drawing vertical lines
        if (coord.x !== null && !firstZ) {
          firstZ = Object.assign({}, coord)
        }
        if (coord.x) {
          lastZ = Object.assign({}, coord)
        }
      }

      // If the first and last coordinates were captured...
      if (firstZ && lastZ) {
        // Create a gradient from point firstZ to lastZ and draw the stroke
        const gradient = ctx.createLinearGradient(
          firstZ.x,
          firstZ.y,
          lastZ.x,
          lastZ.y
        )
        gradient.addColorStop(0, `rgba(${baseRgb}, 1)`)
        gradient.addColorStop(1, `rgba(${baseRgb}, 0)`)
        ctx.strokeStyle = gradient
        ctx.lineWidth = 4
        ctx.moveTo(firstZ.x, firstZ.y)
        ctx.lineTo(lastZ.x, lastZ.y)
        ctx.lineWidth = 1
      }
      ctx.stroke()
      ctx.closePath()
    }

    // HORIZONTAL GRID LINES
    for (let z = zMin; z <= zMax; z += stepSize) {
      let previousPt
      ctx.beginPath()
      let alpha = Math.max(((zMax - z - zMin) / (zMax - zMin)).toFixed(2), 0.2)
      // Step the alpha of the horizontal lines for every z
      ctx.strokeStyle = `rgba(${baseRgb}, ${alpha})`
      for (let x = xMin; x <= xMax; x += stepSize) {
        calibratedPt = new Vector2(x / z, y / z)
        // Passing in true will filter out invalid points
        coord = translator(calibratedPt.x, calibratedPt.y, true)
        // Only draw if the point returned in valid
        if (coord.x != null && coord.y !== null) {
          if (!previousPt) {
            ctx.lineTo(coord.x, coord.y)
          } else {
            ctx.quadraticCurveTo(coord.x, coord.y, coord.x, coord.y)
          }

          previousPt = {
            x: coord.x,
            y: coord.y
          }
        }
      }
      ctx.stroke()
      ctx.closePath()
    }
  }

  drawPointBox = (
    ctx,
    translator,
    size = 1 / 10,
    xMin = -3,
    xMax = 3,
    yMin = -3,
    yMax = 3
  ) => {
    let coord

    ctx.lineWidth = 1
    ctx.strokeStyle = 'lightgrey'
    ctx.fillStyle = 'green'

    // Horizontal
    for (let j = yMin; j < yMax; j += size) {
      ctx.beginPath()
      for (let k = xMin; k < xMax; k += size) {
        coord = translator(k, j)
        ctx.fillRect(coord.x, coord.y, 2, 2)
        ctx.lineTo(coord.x, coord.y)
      }
      ctx.stroke()
      ctx.closePath()
    }

    // Vertical
    for (let k = xMin; k < xMax; k += size) {
      ctx.beginPath()
      for (let j = yMin; j < yMax; j += size) {
        coord = translator(k, j)
        ctx.fillRect(coord.x, coord.y, 2, 2)
        ctx.lineTo(coord.x, coord.y)
      }
      ctx.stroke()
      ctx.closePath()
    }
  }

  calibratedPtTranslator = (x, y, validate = false) => {
    const {
      manifest: { intrinsics: { resolution, focalLength, cameraCenter, kVals } }
    } = this.props

    const calibratedPoint = new Vector2(x, y)
    const result = proj.ProjectCalibratedPointToImage(
      calibratedPoint,
      focalLength,
      cameraCenter,
      kVals
    )
    let pixel = result

    // If we're validating the traversal (projecting and unprojecting), return
    // a null vector if we're beyond the tolerances
    if (validate === true) {
      pixel = this.calibratedValidator(calibratedPoint, result)
      if (pixel.x === null || pixel.y === null) {
        return pixel
      }
    }

    return trans.scaleImageToCanvas(
      pixel,
      resolution.x,
      resolution.y,
      this.canvasWidth,
      this.canvasHeight
    )
  }

  /**
   * Given a calibrated point and a pixel point, use the unproject function to
   * make sure it's a valid point within a certain margin of error; otherwise,
   * return null (and don't render the point)
   * @param  {Object} calibratedPt Vector2 of the calibrated point's (x, y)
   * @param  {Object} pixelPt      Vector2 of the pixel point's (x, y)
   * @param  {Number} tolerance    Tolerance
   * @return {Object}              Vector2 of the pixel point if valid
   */
  calibratedValidator = (calibratedPt, pixelPt, tolerance = 4) => {
    const {
      manifest: { intrinsics: { focalLength, cameraCenter, kVals } }
    } = this.props

    const unprojCalibratedPt = proj.UnprojectImageToCalibratedPoint(
      pixelPt,
      focalLength,
      cameraCenter,
      kVals
    )
    // Check if it the value is within the tolerances
    if (
      Math.abs(unprojCalibratedPt.x) - Math.abs(tolerance) <=
        Math.abs(calibratedPt.x) &&
      Math.abs(calibratedPt.x) <=
        Math.abs(unprojCalibratedPt.x) + Math.abs(tolerance) &&
      Math.abs(unprojCalibratedPt.y) - Math.abs(tolerance) <=
        Math.abs(calibratedPt.y) &&
      Math.abs(calibratedPt.y) <=
        Math.abs(unprojCalibratedPt.y) + Math.abs(tolerance)
    ) {
      return pixelPt
    } else {
      return new Vector2(null, null)
    }
  }

  /**
   * Update all canvases on the component to match the dimensions passed in from
   * props
   */
  readjustCanvasSize = () => {
    const { width, height } = this.props
    this.canvasWidth = width ? width : window.innerWidth
    this.canvasHeight = height ? height : window.innerHeight

    if (this.canvasGndPlane) {
      this.canvasGndPlane.width = width ? width : window.innerWidth
      this.canvasGndPlane.height = height ? height : window.innerHeight
    }
    if (this.canvasDistGrid) {
      this.canvasDistGrid.width = width ? width : window.innerWidth
      this.canvasDistGrid.height = height ? height : window.innerHeight
    }
    if (this.canvasBBox) {
      this.canvasBBox.width = width ? width : window.innerWidth
      this.canvasBBox.height = height ? height : window.innerHeight
    }
    if (this.canvas3dBBox) {
      this.canvas3dBBox.width = width ? width : window.innerWidth
      this.canvas3dBBox.height = height ? height : window.innerHeight
    }
  }

  /**
   * Decodes a bounding box category to a color
   * @param  {String} categoryName Category name
   * @return {String}              Color name
   */
  static decodeBBoxCategoryToColor(categoryName) {
    const name = categoryName ? categoryName.toUpperCase() : ''
    switch (name) {
      case 'PERSON':
        return 'green'
      case 'ANIMAL':
        return 'pink'
      case 'BICYCLE':
        return 'DarkCyan'
      case 'MOTORBIKE':
        return 'GoldenRod'
      case 'CAR':
        return 'DarkOrchid'
      case 'BUS':
        return 'OrangeRed'
      case 'TRUCK':
        return 'LightYellow'
      default:
        return 'white'
    }
  }

  /**
   * Draws a set of bounding boxes on the canvas from the boundingBoxes props
   * (Array)
   */
  drawBoundingBoxes = () => {
    const {
      boundingBoxes,
      manifest: { intrinsics: { resolution } }
    } = this.props
    const INNER_BBOX_OFFSET = 7

    if (this.canvasBBox) {
      // Get the canvas context
      const ctx = this.canvasBBox.getContext('2d')

      // Make sure it's an array before iterating
      if (!Array.isArray(boundingBoxes)) {
        console.log(
          'Cannot render bounding boxes; boundingBoxes is not an array'
        )
        return
      }

      let boundingBox = null
      let color = null
      let canvasCoordTopLeft
      let canvasCoordBottomRight
      let boxWidthInCanvas
      let boxHeightInCanvas
      let label

      for (let k = 0; k < boundingBoxes.length; k++) {
        boundingBox = boundingBoxes[k]
        patchEmptyFields(boundingBox.category, ['type'], 'UNKNOWN')

        // Get the top left (x, y) and convert it to a canvas coordinate
        canvasCoordTopLeft = trans.scaleImageToCanvas(
          new Vector2(boundingBox.topLeftX, boundingBox.topLeftY),
          resolution.x,
          resolution.y,
          this.canvasWidth,
          this.canvasHeight
        )
        // Calculate the bottom right (x, y) and convert it to a canvas
        // coordinate
        canvasCoordBottomRight = trans.scaleImageToCanvas(
          new Vector2(
            boundingBox.topLeftX + boundingBox.extentsX,
            boundingBox.topLeftY + boundingBox.extentsY
          ),
          resolution.x,
          resolution.y,
          this.canvasWidth,
          this.canvasHeight
        )

        // Derive the width of the box in canvas coordinates
        boxWidthInCanvas = Math.abs(
          canvasCoordTopLeft.x - canvasCoordBottomRight.x
        )
        boxHeightInCanvas = Math.abs(
          canvasCoordTopLeft.y - canvasCoordBottomRight.y
        )

        // Draw the outer bounding box with the color varying depending on its
        // category
        color = WebGL.decodeBBoxCategoryToColor(boundingBox.category.type)
        WebGL.drawBoxStroke(
          ctx,
          canvasCoordTopLeft.x,
          canvasCoordTopLeft.y,
          boxWidthInCanvas,
          boxHeightInCanvas,
          color,
          5,
          []
        )

        // Draw a label with the category name on top of the outer bounding
        // box
        label = BBOX_CATEGORY[boundingBox.category.type]
        label += ` (${boundingBox.instanceId ? boundingBox.instanceId : '-1'})`
        WebGL.drawBoxLabel(
          ctx,
          canvasCoordTopLeft.x,
          canvasCoordTopLeft.y - 5,
          color, // Inherits the color from the decode above
          label
        )

        // Draw an inner bounding box that's smaller by a certain offset
        // (INNER_BBOX_OFFSET) and is color-coded depending on the instance ID
        color = COLORS[boundingBox.instanceId % COLORS.length]
        WebGL.drawBoxStroke(
          ctx,
          canvasCoordTopLeft.x + INNER_BBOX_OFFSET,
          canvasCoordTopLeft.y + INNER_BBOX_OFFSET,
          boxWidthInCanvas - INNER_BBOX_OFFSET * 2,
          boxHeightInCanvas - INNER_BBOX_OFFSET * 2,
          color,
          2,
          [5, 1]
        )
      }
    }
  }

  /**
   * Draws a set of 3D bounding boxes on the canvas from the boundingBoxes props
   * (Array)
   */
  drawBoundingBoxes3d = () => {
    const {
      boundingBoxes3d,
      manifest: {
        intrinsics: { resolution, focalLength, cameraCenter, kVals }
      },
      display3dBoundingBoxes
    } = this.props

    if (display3dBoundingBoxes && this.canvas3dBBox) {
      // Get the canvas context
      const ctx = this.canvas3dBBox.getContext('2d')

      const SAMPLE_SIZE = 28 // Should be an even number for the best results

      // Make sure it's an array before iterating
      if (!Array.isArray(boundingBoxes3d)) {
        console.log(
          'Cannot render bounding boxes; boundingBoxes3d is not an array'
        )
        return
      }

      let boundingBox = null
      let color = null
      let label
      let vertices = []
      let calibratedVertices = []
      let scaledVertices = []
      let path

      for (let k = 0; k < boundingBoxes3d.length; k++) {
        boundingBox = boundingBoxes3d[k]

        patchEmptyFields(boundingBox, ['extentsX', 'extentsY', 'extentsZ'])
        patchEmptyFields(boundingBox.pose, [
          'translationX',
          'translationY',
          'translationZ'
        ])

        // Convert meter-space coordinates into the camera space via
        // (x,y,z) => (x/z, y/z)

        // (Near) Top left => top right
        for (
          let n = -boundingBox.extentsX / 2;
          n <= boundingBox.extentsX / 2;
          n += boundingBox.extentsX / SAMPLE_SIZE
        ) {
          vertices.push(
            new Vector2(
              (boundingBox.pose.translationX + n) /
                Math.max(
                  0.25,
                  boundingBox.pose.translationZ - boundingBox.extentsZ / 2
                ),
              (boundingBox.pose.translationY - boundingBox.extentsY / 2) /
                Math.max(
                  0.25,
                  boundingBox.pose.translationZ - boundingBox.extentsZ / 2
                )
            )
          )
        }

        // (Near) Top right => bottom right
        for (
          let n = -boundingBox.extentsY / 2;
          n <= boundingBox.extentsY / 2;
          n += boundingBox.extentsY / SAMPLE_SIZE
        ) {
          vertices.push(
            new Vector2(
              (boundingBox.pose.translationX + boundingBox.extentsX / 2) /
                Math.max(
                  0.25,
                  boundingBox.pose.translationZ - boundingBox.extentsZ / 2
                ),
              (boundingBox.pose.translationY + n) /
                Math.max(
                  0.25,
                  boundingBox.pose.translationZ - boundingBox.extentsZ / 2
                )
            )
          )
        }

        // (Near) Bottom right => bottom left
        for (
          let n = boundingBox.extentsX / 2;
          n >= -boundingBox.extentsX / 2;
          n -= boundingBox.extentsX / SAMPLE_SIZE
        ) {
          vertices.push(
            new Vector2(
              (boundingBox.pose.translationX + n) /
                Math.max(
                  0.25,
                  boundingBox.pose.translationZ - boundingBox.extentsZ / 2
                ),
              (boundingBox.pose.translationY + boundingBox.extentsY / 2) /
                Math.max(
                  0.25,
                  boundingBox.pose.translationZ - boundingBox.extentsZ / 2
                )
            )
          )
        }

        // (Near) Bottom left => top left
        for (
          let n = boundingBox.extentsY / 2;
          n >= -boundingBox.extentsY / 2;
          n -= boundingBox.extentsY / SAMPLE_SIZE
        ) {
          vertices.push(
            new Vector2(
              (boundingBox.pose.translationX - boundingBox.extentsX / 2) /
                Math.max(
                  0.25,
                  boundingBox.pose.translationZ - boundingBox.extentsZ / 2
                ),
              (boundingBox.pose.translationY + n) /
                Math.max(
                  0.25,
                  boundingBox.pose.translationZ - boundingBox.extentsZ / 2
                )
            )
          )
        }

        // (Near) Top left => (Far) Top left
        for (
          let n = -boundingBox.extentsZ / 2;
          n <= boundingBox.extentsZ / 2;
          n += boundingBox.extentsZ / SAMPLE_SIZE
        ) {
          vertices.push(
            new Vector2(
              (boundingBox.pose.translationX - boundingBox.extentsX / 2) /
                Math.max(0.25, boundingBox.pose.translationZ + n),
              (boundingBox.pose.translationY - boundingBox.extentsY / 2) /
                Math.max(0.25, boundingBox.pose.translationZ + n)
            )
          )
        }

        // (Far) Top left => top right
        for (
          let n = -boundingBox.extentsX / 2;
          n <= boundingBox.extentsX / 2;
          n += boundingBox.extentsX / SAMPLE_SIZE
        ) {
          vertices.push(
            new Vector2(
              (boundingBox.pose.translationX + n) /
                Math.max(
                  0.25,
                  boundingBox.pose.translationZ + boundingBox.extentsZ / 2
                ),
              (boundingBox.pose.translationY - boundingBox.extentsY / 2) /
                Math.max(
                  0.25,
                  boundingBox.pose.translationZ + boundingBox.extentsZ / 2
                )
            )
          )
        }
        // are we missing one here?

        // (Far) Top right => (Near) top right => (Far) Top right
        for (
          let n = -boundingBox.extentsZ / 2;
          n <= boundingBox.extentsZ / 2;
          n += boundingBox.extentsZ / SAMPLE_SIZE
        ) {
          vertices.push(
            new Vector2(
              (boundingBox.pose.translationX + boundingBox.extentsX / 2) /
                Math.max(0.25, boundingBox.pose.translationZ + n),
              (boundingBox.pose.translationY - boundingBox.extentsY / 2) /
                Math.max(0.25, boundingBox.pose.translationZ + n)
            )
          )
        }
        for (
          let n = boundingBox.extentsZ / 2;
          n >= -boundingBox.extentsZ / 2;
          n -= boundingBox.extentsZ / SAMPLE_SIZE
        ) {
          vertices.push(
            new Vector2(
              (boundingBox.pose.translationX + boundingBox.extentsX / 2) /
                Math.max(0.25, boundingBox.pose.translationZ + n),
              (boundingBox.pose.translationY - boundingBox.extentsY / 2) /
                Math.max(0.25, boundingBox.pose.translationZ + n)
            )
          )
        }

        // (Far) Top right => bottom right
        for (
          let n = -boundingBox.extentsY / 2;
          n <= boundingBox.extentsY / 2;
          n += boundingBox.extentsY / SAMPLE_SIZE
        ) {
          vertices.push(
            new Vector2(
              (boundingBox.pose.translationX + boundingBox.extentsX / 2) /
                Math.max(
                  0.25,
                  boundingBox.pose.translationZ + boundingBox.extentsZ / 2
                ),
              (boundingBox.pose.translationY + n) /
                Math.max(
                  0.25,
                  boundingBox.pose.translationZ + boundingBox.extentsZ / 2
                )
            )
          )
        }

        // (Far) Bottom right => (Near) Bottom right => (Far) Bottom right
        for (
          let n = -boundingBox.extentsZ / 2;
          n <= boundingBox.extentsZ / 2;
          n += boundingBox.extentsZ / SAMPLE_SIZE
        ) {
          vertices.push(
            new Vector2(
              (boundingBox.pose.translationX + boundingBox.extentsX / 2) /
                Math.max(0.25, boundingBox.pose.translationZ + n),
              (boundingBox.pose.translationY + boundingBox.extentsY / 2) /
                Math.max(0.25, boundingBox.pose.translationZ + n)
            )
          )
        }
        for (
          let n = boundingBox.extentsZ / 2;
          n >= -boundingBox.extentsZ / 2;
          n -= boundingBox.extentsZ / SAMPLE_SIZE
        ) {
          vertices.push(
            new Vector2(
              (boundingBox.pose.translationX + boundingBox.extentsX / 2) /
                Math.max(0.25, boundingBox.pose.translationZ + n),
              (boundingBox.pose.translationY + boundingBox.extentsY / 2) /
                Math.max(0.25, boundingBox.pose.translationZ + n)
            )
          )
        }

        // (Far) Bottom right => bottom left
        for (
          let n = boundingBox.extentsX / 2;
          n >= -boundingBox.extentsX / 2;
          n -= boundingBox.extentsX / SAMPLE_SIZE
        ) {
          vertices.push(
            new Vector2(
              (boundingBox.pose.translationX + n) /
                Math.max(
                  0.25,
                  boundingBox.pose.translationZ + boundingBox.extentsZ / 2
                ),
              (boundingBox.pose.translationY + boundingBox.extentsY / 2) /
                Math.max(
                  0.25,
                  boundingBox.pose.translationZ + boundingBox.extentsZ / 2
                )
            )
          )
        }

        // (Far) Bottom left => (Near) Bottom left => (Far) Bottom left
        for (
          let n = -boundingBox.extentsZ / 2;
          n <= boundingBox.extentsZ / 2;
          n += boundingBox.extentsZ / SAMPLE_SIZE
        ) {
          vertices.push(
            new Vector2(
              (boundingBox.pose.translationX - boundingBox.extentsX / 2) /
                Math.max(0.25, boundingBox.pose.translationZ + n),
              (boundingBox.pose.translationY + boundingBox.extentsY / 2) /
                Math.max(0.25, boundingBox.pose.translationZ + n)
            )
          )
        }
        for (
          let n = boundingBox.extentsZ / 2;
          n >= -boundingBox.extentsZ / 2;
          n -= boundingBox.extentsZ / SAMPLE_SIZE
        ) {
          vertices.push(
            new Vector2(
              (boundingBox.pose.translationX - boundingBox.extentsX / 2) /
                Math.max(0.25, boundingBox.pose.translationZ + n),
              (boundingBox.pose.translationY + boundingBox.extentsY / 2) /
                Math.max(0.25, boundingBox.pose.translationZ + n)
            )
          )
        }

        // (Far) Bottom left => top left
        for (
          let n = boundingBox.extentsY / 2;
          n >= -boundingBox.extentsY / 2;
          n -= boundingBox.extentsY / SAMPLE_SIZE
        ) {
          vertices.push(
            new Vector2(
              (boundingBox.pose.translationX - boundingBox.extentsX / 2) /
                Math.max(
                  0.25,
                  boundingBox.pose.translationZ + boundingBox.extentsZ / 2
                ),
              (boundingBox.pose.translationY + n) /
                Math.max(
                  0.25,
                  boundingBox.pose.translationZ + boundingBox.extentsZ / 2
                )
            )
          )
        }

        calibratedVertices = vertices.map((v, i) =>
          proj.ProjectCalibratedPointToImage(
            v,
            focalLength,
            cameraCenter,
            kVals
          )
        )

        scaledVertices = calibratedVertices.map((v, i) =>
          trans.scaleImageToCanvas(
            v,
            resolution.x,
            resolution.y,
            this.canvasWidth,
            this.canvasHeight
          )
        )

        // Draw the outer bounding box with the color varying depending on its
        // category
        color = WebGL.decodeBBoxCategoryToColor(boundingBox.category.type)
        path = new Path2D()
        scaledVertices.forEach(v => path.lineTo(v.x, v.y))

        ctx.lineWidth = 2
        ctx.strokeStyle = color
        ctx.stroke(path)

        // Draw a label with the category name on top of the outer bounding
        // box
        label = BBOX_CATEGORY[boundingBox.category.type]
          ? BBOX_CATEGORY[boundingBox.category.type]
          : 'Unknown'
        label += ` (${boundingBox.instanceId})`
        WebGL.drawBoxLabel(
          ctx,
          scaledVertices[0].x,
          scaledVertices[0].y - 10,
          color, // Inherits the color from the decode above
          label
        )

        // Clear the array for the next bounding box
        vertices = []
      }
    }
  }

  /**
   * Draws a convex hull around objects
   * @return {[type]} [description]
   */
  drawConvexHulls = () => {
    const {
      convexHulls,
      manifest: {
        intrinsics: { resolution, focalLength, cameraCenter, kVals }
      },
      displayConvexHulls
    } = this.props

    if (displayConvexHulls && this.canvas3dBBox) {
      // Get the canvas context
      const ctx = this.canvas3dBBox.getContext('2d')

      // Make sure it's an array before iterating
      if (!Array.isArray(convexHulls)) {
        console.log('Cannot render convex hulls; convexHulls is not an array')
        return
      }

      let convexHull
      let color
      let path
      let label
      let hullUpperVertices = []
      let hullLowerVertices = []
      let hullEdges = []

      for (let k = 0; k < convexHulls.length; k++) {
        convexHull = convexHulls[k]

        patchEmptyFields(convexHulls, ['xs', 'extentsY', 'zs'])
        patchEmptyFields(convexHulls.pose, [
          'translationX',
          'translationY',
          'translationZ'
        ])

        for (let n = 0; n < convexHull.xs.length; n++) {
          let x = convexHull.xs[n]
          let z = convexHull.zs[n]
          let upper = new Vector2(
            x / z,
            convexHull.pose.translationY - convexHull.extentsY / z
          )
          let lower = new Vector2(
            x / z,
            convexHull.pose.translationY + convexHull.extentsY / z
          )

          hullUpperVertices.push(upper)
          hullLowerVertices.push(lower)
          hullEdges.push(upper)
          hullEdges.push(lower)
        }

        hullUpperVertices = hullUpperVertices
          .map((v, i) =>
            proj.ProjectCalibratedPointToImage(
              v,
              focalLength,
              cameraCenter,
              kVals
            )
          )
          .map((v, i) =>
            trans.scaleImageToCanvas(
              v,
              resolution.x,
              resolution.y,
              this.canvasWidth,
              this.canvasHeight
            )
          )

        hullLowerVertices = hullLowerVertices
          .map((v, i) =>
            proj.ProjectCalibratedPointToImage(
              v,
              focalLength,
              cameraCenter,
              kVals
            )
          )
          .map((v, i) =>
            trans.scaleImageToCanvas(
              v,
              resolution.x,
              resolution.y,
              this.canvasWidth,
              this.canvasHeight
            )
          )

        hullEdges = hullEdges
          .map((v, i) =>
            proj.ProjectCalibratedPointToImage(
              v,
              focalLength,
              cameraCenter,
              kVals
            )
          )
          .map((v, i) =>
            trans.scaleImageToCanvas(
              v,
              resolution.x,
              resolution.y,
              this.canvasWidth,
              this.canvasHeight
            )
          )

        color = WebGL.decodeBBoxCategoryToColor(convexHull.category.type)
        ctx.lineWidth = 2
        ctx.strokeStyle = color

        // Upper
        path = new Path2D()
        hullUpperVertices.forEach(v => path.lineTo(v.x, v.y))
        path.closePath()
        ctx.stroke(path)

        // Lower
        path = new Path2D()
        hullLowerVertices.forEach(v => path.lineTo(v.x, v.y))
        path.closePath()
        ctx.stroke(path)

        // Edges
        path = new Path2D()
        for (let n = 0; n < hullEdges.length; n += 2) {
          let u = hullEdges[n]
          let v = hullEdges[n + 1]
          path.moveTo(u.x, u.y)
          path.lineTo(v.x, v.y)
        }
        ctx.stroke(path)

        // Draw a label with the category name on top of the outer bounding
        // box
        label = BBOX_CATEGORY[convexHull.category.type]
          ? BBOX_CATEGORY[convexHull.category.type]
          : 'Unknown'
        label += ` (${convexHull.instanceId})`
        WebGL.drawBoxLabel(
          ctx,
          hullUpperVertices[0].x,
          hullUpperVertices[0].y - 10,
          color, // Inherits the color from the decode above
          label
        )

        // Clear the array for the next bounding box
        hullEdges = []
        hullUpperVertices = []
        hullLowerVertices = []
      }
    }
  }

  /**
   * Draws a box on the given canvas context
   * @param  {Object} ctx       Canvas context
   * @param  {Number} x         x image coordinate
   * @param  {Number} y         y image coordinate
   * @param  {Number} width     Width in image coordinates
   * @param  {Number} height    Height in image coordinates
   * @param  {String} color     Color of the box's stroke
   * @param  {Number} lineWidth Line width
   * @param  {Array}  dash      Dash pattern for the stroke
   */
  static drawBoxStroke(
    ctx,
    x,
    y,
    width,
    height,
    color = 'white',
    lineWidth = 5,
    dash = []
  ) {
    ctx.strokeStyle = color ? color : 'white'
    ctx.lineWidth = lineWidth
    ctx.setLineDash(dash)
    ctx.strokeRect(x, y, width, height)
  }

  /**
   * Draws a label above the bounding box
   * @param  {Object} ctx      Canvas context
   * @param  {Number} x        x image coordinate
   * @param  {Number} y        y image coordinate
   * @param  {String} color    Color for text
   * @param  {String} text     Text to display
   */
  static drawBoxLabel(ctx, x, y, color = null, text = '') {
    ctx.fillStyle = color
    ctx.font = '14px Helvetica, serif'
    ctx.fillText(text, x, y)
  }

  render() {
    const {
      top,
      left,
      width,
      height,
      visible,
      displayGroundPlane,
      displayDistortionGrid,
      displayBoundingBoxes,
      display3dBoundingBoxes
    } = this.props

    return (
      <Container
        visible={visible}
        top={top}
        left={left}
        width={width}
        height={height}
      >
        <canvas
          name="bounding-boxes"
          ref={c => (this.canvasBBox = c)}
          style={{ display: displayBoundingBoxes ? 'block' : 'none' }}
        />
        <canvas
          name="3d-bounding-boxes"
          ref={c => (this.canvas3dBBox = c)}
          style={{ display: display3dBoundingBoxes ? 'block' : 'none' }}
        />
        <canvas
          name="ground-plane"
          ref={c => (this.canvasGndPlane = c)}
          style={{ display: displayGroundPlane ? 'block' : 'none' }}
        />
        <canvas
          name="distortion-grid"
          ref={c => (this.canvasDistGrid = c)}
          style={{ display: displayDistortionGrid ? 'block' : 'none' }}
        />
      </Container>
    )
  }
}

export default WebGL
