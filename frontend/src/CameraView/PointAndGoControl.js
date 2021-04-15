import React from 'react'
import PropTypes from 'prop-types'
import styled from 'styled-components'
import * as pag from '../utils/pointandgo'
import { Vector2 } from '../utils/vectors'

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

const SVGContainer = styled.div`
  position: relative;
  top: 0;
  left: 0;
  width: 125px;
  height: 125px;
  transition: 0.2s all;
  opacity: 0;
  display: none;
`

const Angle = styled.div`
  position: absolute;
  top: -10px;
  left: -20px;
  padding: 4px 6px;
  background-color: rgba(0, 0, 0, 0.7);
  color: #fff;
  font-size: 10px;
  border-radius: 3px;
`

/**
 * Point and Go Control
 * =============================================================================
 * This component encapsulates the point and go functionality for the
 * TeleopUI. It will draw an approximate final location of the vehicle on the
 * ground wrt the distorted field and will allow the user to choose an
 * orientation after the move command has been made.
 *
 * The component will receive a top, left, width, and height prop from its
 * parent component, which will determine the size of the drawable canvas.
 * Usually, these are taken from the bounding box dimensions of the <video>
 * element.
 */
class PointAndGoControl extends React.Component {
  static propTypes = {
    top: PropTypes.number,
    left: PropTypes.number,
    width: PropTypes.number,
    height: PropTypes.number,
    ignoreModifier: PropTypes.string,
    onPointAndGo: PropTypes.func.isRequired,
    manifest: PropTypes.object.isRequired
  }

  static defaultProps = {
    ignoreModifier: null
  }

  constructor(props) {
    super(props)

    this.state = {
      angle: 0,
      selected: {
        x: null,
        y: null
      }
    }
  }

  componentDidMount = () => {
    this.resize()
  }

  shouldComponentUpdate(nextProps, nextState) {
    // Don't trigger rerenders unless any of the below props have been changed
    const { top, left, width, height } = this.props
    const { angle } = this.state
    const {
      top: nextTop,
      left: nextLeft,
      width: nextWidth,
      height: nextHeight
    } = nextProps
    const { angle: nextAngle } = nextState

    const update = !(
      Math.floor(top) === Math.floor(nextTop) &&
      Math.floor(left) === Math.floor(nextLeft) &&
      Math.floor(width) === Math.floor(nextWidth) &&
      Math.floor(height) === Math.floor(nextHeight) &&
      Math.floor(angle) === Math.floor(nextAngle)
    )

    return update
  }

  clearCanvas = ctx => {
    ctx.clearRect(0, 0, this.dynamicCanvas.width, this.dynamicCanvas.height)
  }

  showAngleAdjuster = (x, y) => {
    const { width, height } = this.svgContainer.getBoundingClientRect()
    // yOffset so the adjuster doesn't display directly on the cursor
    const yOffset = 120

    this.svgContainer.style.display = 'block'
    this.svgContainer.style.opacity = 1
    this.svgContainer.style.top = `${y - height / 2 - yOffset}px`
    this.svgContainer.style.left = `${x - width / 2}px`
  }

  handleClick = evt => {
    this.showAngleAdjuster(evt.clientX, evt.clientY)

    const { manifest: { intrinsics } } = this.props
    const canvasDimensions = {
      x: this.canvasWidth,
      y: this.canvasHeight
    }
    const ctx = this.dynamicCanvas.getContext('2d')
    const mousePt = new Vector2(
      evt.nativeEvent.offsetX,
      evt.nativeEvent.offsetY
    )

    this.setState(() => {
      // Clear previously drawn rects
      this.clearCanvas(ctx)
      pag.drawDistortedRect(ctx, mousePt, 0, intrinsics, canvasDimensions)

      return {
        selected: {
          x: mousePt.x,
          y: mousePt.y
        }
      }
    })
  }

  handleMouseMove = evt => {
    const { manifest: { intrinsics } } = this.props
    const canvasDimensions = {
      x: this.canvasWidth,
      y: this.canvasHeight
    }
    const ctx = this.dynamicCanvas.getContext('2d')
    this.clearCanvas(ctx)
    let mousePt

    if (this.state.selected.x === null || this.state.selected.y === null) {
      mousePt = new Vector2(evt.nativeEvent.offsetX, evt.nativeEvent.offsetY)
      pag.drawDistortedRect(ctx, mousePt, 0, intrinsics, canvasDimensions)
    } else {
      // Center of vehicle
      mousePt = new Vector2(this.state.selected.x, this.state.selected.y)

      const angle =
        Math.atan2(
          Math.floor(mousePt.y) - evt.clientY,
          Math.floor(mousePt.x) - evt.clientX
        ) *
        180 /
        Math.PI

      this.setState(() => {
        this.svg.style.transform = `rotate(${angle - 270}deg)`
        return { angle: Math.floor(angle) - 90 }
      })

      pag.drawDistortedRect(
        ctx,
        mousePt,
        -(angle - 90) * Math.PI / 180, // Degrees => radians
        intrinsics,
        canvasDimensions,
        true
      )
    }
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
    const { angle } = this.state

    return (
      <Container
        top={top}
        left={left}
        width={width}
        height={height}
        onClick={this.handleClick}
        onMouseMove={this.handleMouseMove}
      >
        <canvas name="dynamic-canvas" ref={c => (this.dynamicCanvas = c)} />
        <SVGContainer innerRef={r => (this.svgContainer = r)}>
          <Angle>{angle}&deg;</Angle>
          <svg
            height="125"
            width="125"
            viewBox="0 0 220 220"
            ref={r => (this.svg = r)}
          >
            <circle
              cx="110"
              cy="110"
              r="100"
              stroke="#ccc"
              fill="none"
              strokeWidth="3"
              strokeDasharray="1 8.85"
            />
            <circle
              cx="110"
              cy="110"
              r="100"
              stroke="#ccc"
              fill="none"
              strokeWidth="7"
              strokeDasharray="1 38.4"
            />
            <circle
              cx="110"
              cy="110"
              r="78"
              stroke="#ccc"
              fill="none"
              strokeWidth="2"
            />
            <circle cx="110" cy="110" r="4" fill="#FE7867" />
            <rect
              x="67.5"
              y="67.5"
              width="85"
              height="85"
              rx="5"
              ry="5"
              stroke="#ccc"
              fill="none"
              strokeWidth="2"
            />
            <line
              x1="110"
              y1="68"
              x2="110"
              y2="0"
              strokeWidth="2"
              stroke="#ccc"
            />
            <line
              x1="105"
              y1="1"
              x2="115"
              y2="1"
              strokeWidth="2"
              stroke="#ccc"
            />
            <polygon
              points="100 162 120 162 110 170"
              stroke="none"
              fill="#F5A623"
            />
          </svg>
        </SVGContainer>
      </Container>
    )
  }
}

export default PointAndGoControl
