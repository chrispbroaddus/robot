import React from 'react'
import PropTypes from 'prop-types'
import styled from 'styled-components'
import Modal from 'react-modal'
import Close from 'react-icons/lib/md/close'
import ZStageBody from './Body'
import ZStageLeg from './Leg'
import { scaleAtoB } from '../../utils/translations'

const Container = styled.div`
  position: absolute;
  bottom: 25px;
  left: 40px;
  z-index: 1000;
  padding: 15px;
  border-radius: 20px;
  background-color: rgba(0, 0, 0, 0.5);
`
const ModalContainer = styled.div`
  position: relative;
  margin: auto;
  user-select: none;
`
const CloseButton = styled(Close)`
  color: #fe7867;
  font-size: 15px;
  position: absolute;
  top: -5px;
  right: -5px;
  cursor: pointer;
`
const Title = styled.h5`
  text-transform: uppercase;
  letter-spacing: 2px;
  color: white;
  margin: 0 0 15px 0;
  padding: 0 10px;
`
const Zippy = styled.div`
  height: 355px;
  padding: 0 35px;
  display: flex;
  flex-direction: row;
  justify-content: center;
  align-items: flex-end;
`
const Leg = styled(ZStageLeg)`display: inline-block;`
const BodyContainer = styled.div`
  height: 300px;
  & > svg {
    cursor: grab;
    // transition: 0.1s;
    -webkit-backface-visibility: hidden;
    position: relative;
    top: 0;
    left: 0;
  }
`

const HeightIndicator = styled.div`
  color: white;
  text-transform: uppercase;
  text-align: center;
  position: relative;
  top -15px;
  font-size: 11px;

  & > span {
    color: #00E4B9;
  }
`

const Instructions = styled.div`
  font-size: 12px;
  color: white;
  text-align: center;
  margin: 15px 0 7px;
`

// Adjust the below to change the effective range of the mousemove
const containerMin = 30
const containerMax = 200
// Adjust the below to change the maximum up/down offsets for the robot
// SVG
const pictureMin = -75
const pictureMax = 40
// Adjust the below to change the minimum/maximum values that the vehicle
// can accept
const vehicleMin = 0
const vehicleMax = 0.4
// Adjust the below to change the minimum/maximum height from the ground
const groundHeightMin = 5.5
const groundHeightMax = 21.5

class ZStageContainer extends React.PureComponent {
  static propTypes = {
    onSendValue: PropTypes.func.isRequired,
    isOpen: PropTypes.bool,
    onRequestClose: PropTypes.func
  }

  static defaultProps = {
    isOpen: false,
    onRequestClose: () => {}
  }

  constructor(props) {
    super(props)

    this.mouseDown = false
    this.state = {
      zStageValue: vehicleMin.toFixed(2),
      groundHeight: groundHeightMin.toFixed(1)
    }
  }

  handleMouseDown = evt => {
    this.mouseDown = true
    this.bodySvgRef.style.cursor = '-webkit-grabbing'
  }

  handleMouseUp = () => {
    const { onSendValue } = this.props
    const { zStageValue } = this.state

    this.mouseDown = false
    this.bodySvgRef.style.cursor = '-webkit-grab'
    onSendValue(zStageValue)
  }

  handleMouseMove = evt => {
    if (this.mouseDown === true) {
      this.bodySvgRef.style.cursor = '-webkit-grabbing'
      const container = this.bodyContainerRef.getBoundingClientRect()
      // const body = this.bodySvgRef.getBoundingClientRect()
      // console.log(`Cont:   left\t${container.left}, top\t${container.top} `)
      // console.log(`Body:   left\t${body.left}, top\t${body.top} `)
      // console.log(`Mouse:  x\t${evt.clientX}, y\t\t${evt.clientY}`)
      const delta = evt.clientY - container.top
      // console.log(`ydelta ${delta}`)

      if (delta > containerMin && delta < containerMax) {
        const bodyTop = scaleAtoB(
          delta,
          containerMin,
          containerMax,
          pictureMin,
          pictureMax
        )
        this.bodySvgRef.style.top = `${Math.ceil(bodyTop)}px`

        const zStage = scaleAtoB(
          Math.ceil(bodyTop),
          pictureMin,
          pictureMax,
          vehicleMax,
          vehicleMin
        )

        const groundHeight = scaleAtoB(
          zStage,
          vehicleMin,
          vehicleMax,
          groundHeightMin,
          groundHeightMax
        )

        this.setState({
          zStageValue: zStage.toFixed(2),
          groundHeight: groundHeight.toFixed(1)
        })
      } else if (delta < containerMin) {
        // If the mouse is at the extremes, assume the max/min values
        this.setState({
          zStageValue: vehicleMax.toFixed(2),
          groundHeight: groundHeightMax.toFixed(1)
        })
        this.bodySvgRef.style.top = `${pictureMin}px`
      } else if (delta > containerMin) {
        this.setState({
          zStageValue: vehicleMin.toFixed(2),
          groundHeight: groundHeightMin.toFixed(1)
        })
        this.bodySvgRef.style.top = `${pictureMax}px`
      }
    }
  }

  render() {
    const { isOpen, onRequestClose } = this.props
    const { groundHeight } = this.state

    return (
      <Container>
        <Modal
          isOpen={isOpen}
          onRequestClose={onRequestClose}
          className="zstage-modal"
          overlayClassName="modal-overlay"
          contentLabel="Z Stage"
        >
          <ModalContainer>
            <CloseButton onClick={onRequestClose} />
            <Title>Z-Stage</Title>
            <Zippy>
              <Leg />
              <BodyContainer
                onMouseDown={this.handleMouseDown}
                onMouseUp={this.handleMouseUp}
                innerRef={r => (this.bodyContainerRef = r)}
              >
                <ZStageBody
                  onMouseMove={this.handleMouseMove}
                  innerRef={r => (this.bodySvgRef = r)}
                />
              </BodyContainer>
              <Leg />
            </Zippy>
            <HeightIndicator>
              <span>{groundHeight}</span> in
            </HeightIndicator>
            <Instructions>Drag center of robot to position</Instructions>
          </ModalContainer>
        </Modal>
      </Container>
    )
  }
}

export default ZStageContainer
