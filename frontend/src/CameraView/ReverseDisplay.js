import React from 'react'
import styled, { keyframes } from 'styled-components'
import PropTypes from 'prop-types'
import * as vehicleDucks from '../redux/modules/vehicles'

const widthPx = 48
const heightPx = 56

const pulse = keyframes`
  0% {
    opacity: 1;
  }
  50% {
    opacity: 0.5;
  }
  100% {
    opacity: 1;
  }
`

const Container = styled.div`
  position: absolute;
  right: 42px;
  bottom: 20px;
  width: ${widthPx}px;
  height: ${heightPx}px;
  // The z-index of this container should be higher than the CommandText
  // component. Otherwise, we will have an unclickable dead zone in the middle
  // of the container
  z-index: 1000;
  cursor: pointer;
  & > * {
    cursor: pointer;
  }
`

const CommandChiclet = styled.div`
  position: absolute;
  font-size: 1.5rem;
  letter-spacing: 1px;
  color: white;
  width: ${widthPx}px;
  height: ${heightPx}px;
  border-radius: 5.64px;
  text-align: center;
  line-height: ${heightPx}px;
  user-select: none;

  ${props =>
    props.pulse &&
    `
      color: #f5a623;
      animation: ${pulse} 2s linear infinite;
    `};
`

const ImageContainer = styled.div`
  position: absolute;
  top: 0;
  left: 0;
  & > svg polygon {
    stroke: #f9f9f9;
  }
  ${props =>
    props.pulse &&
    `
      & > svg polygon {
        stroke: #f5a623;
        animation: ${pulse} 2s linear infinite;
      }
    `};
`

const MenuContainer = styled.div`
  position: absolute;
  bottom: 90px;
  right: 0;
  border-radius: 13px;
  background-color: #000000;
  color: #fff;
  padding: 22px 27px;
  min-width: 165px;
  transition: 0.2s all;
  opacity: ${props => (props.visible ? 0.7 : 0)};
  visibility: ${props => (props.visible ? 'visible' : 'hidden')};
`

const Item = styled.div`
  font-size: 12px;
  color: ${props => (props.selected ? '#fe7867' : '#fff')};
  margin-bottom: 11px;
  transition: 0.2s;
`

const validRoles = Object.keys(vehicleDucks.CAMERA_ROLES).map(v =>
  Symbol.keyFor(vehicleDucks.CAMERA_ROLES[v])
)

class ReverseDisplay extends React.Component {
  static propTypes = {
    cameraRole: PropTypes.oneOf(validRoles).isRequired,
    onClick: PropTypes.func.isRequired,
    disabled: PropTypes.bool
  }

  static defaultProps = {
    disabled: false
  }

  constructor(props) {
    super(props)

    this.state = {
      displayMenu: false
    }
  }

  closeMenu = () => {
    this.setState(prevState => ({ displayMenu: false }))
  }

  handleClick = () => {
    this.setState(prevState => ({
      displayMenu: !prevState.displayMenu
    }))
  }

  handleClickMenuItem = (evt, cameraRole) => {
    this.props.onClick(cameraRole) // Should call the camera switcher from the parent
    this.closeMenu()
    evt.stopPropagation()
  }

  render() {
    const { cameraRole } = this.props
    const { displayMenu } = this.state
    const { isRearCamera, CAMERA_ROLES } = vehicleDucks

    const isRear = isRearCamera(cameraRole)
    return (
      <Container onClick={this.handleClick}>
        <ImageContainer pulse={isRear}>
          <svg width="48px" height="54px" viewBox="0 0 48 54">
            <g
              id="Reverse-and-Camera-Switch"
              stroke="none"
              strokeWidth="1"
              fill="none"
              fillRule="evenodd"
            >
              <g
                id="camera-switcher"
                transform="translate(-1350.000000, -815.000000)"
              >
                <g
                  id="keybind-hexagon"
                  transform="translate(1359.000000, 827.000000)"
                >
                  <polygon
                    className="hexagon"
                    strokeWidth="2"
                    strokeLinejoin="round"
                    points="14.5166605 -11 37.033321 2 37.033321 28 14.5166605 41 -8 28 -8 2"
                  />
                </g>
              </g>
            </g>
          </svg>
        </ImageContainer>

        <CommandChiclet pulse={isRearCamera(cameraRole)}>
          {!isRear && 'F'}
          {isRear && 'R'}
        </CommandChiclet>
        <MenuContainer visible={displayMenu}>
          <Item
            selected={!isRear}
            onClick={evt =>
              this.handleClickMenuItem(
                evt,
                Symbol.keyFor(CAMERA_ROLES.FrontFisheye)
              )}
          >
            front camera
          </Item>
          <Item
            selected={isRear}
            onClick={evt =>
              this.handleClickMenuItem(
                evt,
                Symbol.keyFor(CAMERA_ROLES.RearFisheye)
              )}
          >
            reverse camera
          </Item>
        </MenuContainer>
      </Container>
    )
  }
}

export default ReverseDisplay
