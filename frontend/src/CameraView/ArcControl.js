import React from 'react'
import styled from 'styled-components'
import PropTypes from 'prop-types'
import * as arc from '../utils/arc'
import * as pag from '../utils/pointandgo'
import { Vector2 } from '../utils/vectors'

// Number of samples grab from the trajectory (higher = better resolution
// of curves)
export const ARC_SAMPLE_SIZE = 100
export const VEHICLE_WIDTH = 1 // In meters
export const CAMERA_HEIGHT = 1 // In meters

const Container = styled.div`
  position: absolute;
  top: ${props => (props.top ? `${props.top}px` : 0)};
  left: ${props => (props.left ? `${props.left}px` : 0)};
  width: ${props => (props.width ? `${props.width}px` : '100%')};
  height: ${props => (props.height ? `${props.height}px` : '100%')};
  z-index: 500;
  user-select: none;

  & > canvas {
    position: absolute;
    top: 0;
    left: 0;
  }
`

class Arc extends React.Component {
  static propTypes = {
    top: PropTypes.number,
    left: PropTypes.number,
    width: PropTypes.number,
    height: PropTypes.number,
    manifest: PropTypes.object.isRequired,
    ignoreModifier: PropTypes.string,
    onArc: PropTypes.func.isRequired
  }

  static defaultProps = {
    ignoreModifier: null
  }

  constructor(props) {
    super(props)

    this.state = {
      mouseDown: false,
      selected: {
        x: null,
        y: null
      },
      angle: 0.0
    }
  }

  componentDidMount() {
    this.resize()
  }

  componentWillUnmount() {}

  shouldComponentUpdate(nextProps) {
    // Don't trigger rerenders unless any of the below props have been changed
    const { top, left, width, height, manifest } = this.props
    const {
      top: nextTop,
      left: nextLeft,
      width: nextWidth,
      height: nextHeight,
      manifest: nextManifest
    } = nextProps

    const update = !(
      Math.floor(top) === Math.floor(nextTop) &&
      Math.floor(left) === Math.floor(nextLeft) &&
      Math.floor(width) === Math.floor(nextWidth) &&
      Math.floor(height) === Math.floor(nextHeight) &&
      manifest == nextManifest
    )
    return update
  }

  clearCanvas = ctx => {
    ctx.clearRect(0, 0, this.dynamicCanvas.width, this.dynamicCanvas.height)
  }

  handleClick = evt => {
    const { manifest: { intrinsics } } = this.props
    const { manifest: { intrinsics: { resolution } } } = this.props
    const { angle } = this.state
    const canvasDimensions = {
      x: this.canvasWidth,
      y: this.canvasHeight
    }
    const ctx = this.dynamicCanvas.getContext('2d')
    this.clearCanvas(ctx)

    // If an ignore modifier has been specified on this component and the
    // modifier has been pressed in combination with a click, ignore this event
    if (
      this.props.ignoreModifier !== null &&
      evt.nativeEvent[this.props.ignoreModifier] === true
    ) {
      return
    }

    const mousePt = new Vector2(
      evt.nativeEvent.offsetX,
      evt.nativeEvent.offsetY
    )

    arc.drawArcs(
      ctx,
      mousePt,
      intrinsics,
      canvasDimensions,
      CAMERA_HEIGHT,
      ARC_SAMPLE_SIZE,
      VEHICLE_WIDTH
    )

    pag.drawDistortedRect(
      ctx,
      mousePt,
      angle * Math.PI / 180,
      intrinsics,
      canvasDimensions,
      true
    )

    const { x: percentX, y: percentY } = arc.canvasPointToImagePercentage(
      mousePt,
      resolution,
      canvasDimensions
    )
    this.props.onArc(
      parseFloat(percentX.toFixed(5)),
      parseFloat(percentY.toFixed(5)),
      parseFloat(angle.toFixed(2))
    )
  }

  handleMouseMove = evt => {
    const { manifest: { intrinsics } } = this.props
    const canvasDimensions = {
      x: this.canvasWidth,
      y: this.canvasHeight
    }
    const ctx = this.dynamicCanvas.getContext('2d')
    let mousePt

    if (this.state.mouseDown === false) {
      // If the left mouse button is not depressed, draw the arc and box
      // normally
      mousePt = new Vector2(evt.nativeEvent.offsetX, evt.nativeEvent.offsetY)

      this.clearCanvas(ctx)
      arc.drawArcs(
        ctx,
        mousePt,
        intrinsics,
        canvasDimensions,
        CAMERA_HEIGHT,
        ARC_SAMPLE_SIZE,
        VEHICLE_WIDTH
      )

      const calibratedPt = pag.canvasPointToCalibratedPoint(
        mousePt,
        intrinsics,
        canvasDimensions
      )

      const { x, z } = pag.calibratedPointToCameraPoint(calibratedPt)
      const r = (x * x + z * z) / (2 * x)
      let theta
      if (r > 0) {
        theta = z < x ? Math.PI - Math.asin(-z / r) : Math.asin(-z / r)
      } else {
        theta = z < -x ? Math.PI - Math.asin(-z / r) : Math.asin(-z / r)
      }

      pag.drawDistortedRect(
        ctx,
        mousePt,
        theta,
        intrinsics,
        canvasDimensions,
        true
      )

      this.setState({ angle: theta * 180 / Math.PI })
    } else {
      // If the left mouse button IS depressed, check if the mouse position is
      // outside of the "dead zone" before drawing the arc in a static position
      // and rotating the box
      const DEAD_ZONE_LENGTH = 25 // In pixels
      if (
        (evt.nativeEvent.offsetX >
          this.state.selected.x - DEAD_ZONE_LENGTH / 2 &&
          this.state.selected.x + DEAD_ZONE_LENGTH / 2 >
            evt.nativeEvent.offsetX) ||
        (evt.nativeEvent.offsetY >
          this.state.selected.y - DEAD_ZONE_LENGTH / 2 &&
          this.state.selected.y + DEAD_ZONE_LENGTH / 2 >
            evt.nativeEvent.offsetY)
      ) {
        // Mouse position is in the dead zone; do nothing
        return
      }

      mousePt = new Vector2(this.state.selected.x, this.state.selected.y)

      // Finds the angle between the x-axis and a line from x to mousePt (in
      // degrees)
      const angle =
        Math.atan2(
          Math.floor(mousePt.y) - evt.nativeEvent.offsetY,
          Math.floor(mousePt.x) - evt.nativeEvent.offsetX
        ) *
        180 /
        Math.PI

      this.clearCanvas(ctx)
      arc.drawArcs(
        ctx,
        mousePt,
        intrinsics,
        canvasDimensions,
        CAMERA_HEIGHT,
        ARC_SAMPLE_SIZE,
        VEHICLE_WIDTH
      )
      pag.drawDistortedRect(
        ctx,
        mousePt,
        -(angle - 90) * Math.PI / 180, // Degrees => radians
        intrinsics,
        canvasDimensions,
        true
      )
      this.setState({ angle: angle - 90 })
    }
  }

  handleMouseDown = evt => {
    this.dynamicCanvas.style.cursor = 'move'
    this.setState({
      mouseDown: true,
      selected: {
        x: evt.nativeEvent.offsetX,
        y: evt.nativeEvent.offsetY
      }
    })
  }

  handleMouseUp = evt => {
    this.dynamicCanvas.style.cursor = 'auto'
    this.setState({
      mouseDown: false,
      selected: {
        x: null,
        y: null
      }
    })
  }

  resize = () => {
    const { width, height } = this.props
    this.canvasWidth = width ? width : window.innerWidth
    this.canvasHeight = height ? height : window.innerHeight

    if (this.dynamicCanvas) {
      this.dynamicCanvas.width = width ? width : window.innerWidth
      this.dynamicCanvas.height = height ? height : window.innerHeight
    }
  }

  render() {
    const { width, height, top, left } = this.props

    // Resize the canvas on redraw
    this.resize()

    return (
      <Container
        top={top}
        left={left}
        width={width}
        height={height}
        onMouseMove={this.handleMouseMove}
        onMouseDown={this.handleMouseDown}
        onMouseUp={this.handleMouseUp}
        onClick={this.handleClick}
      >
        <canvas name="dynamic-canvas" ref={c => (this.dynamicCanvas = c)} />
      </Container>
    )
  }
}

export default Arc
