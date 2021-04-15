import React from 'react'
import PropTypes from 'prop-types'
import styled from 'styled-components'
import { bindActionCreators } from 'redux'
import { push } from 'react-router-redux'
import { connect } from 'react-redux'
import './docking-ui.styles.scss'
import * as vehicleDucks from '../../redux/modules/vehicles'
import DockingAnimation from './DockingAnimation'
import TelemetryData from './DockingAnimation/TelemetryData.js'

const Container = styled.div`
  position: absolute;
  right: 55px;
  top: 260px;
  z-index: 500;
`

class Docking extends React.Component {
  dock

  static propTypes = {
    message: PropTypes.object,
    angle: PropTypes.number,
    distanceX: PropTypes.number,
    distanceY: PropTypes.number,
    status: PropTypes.string,
    setDockingDivLocation: PropTypes.func.isRequired
  }

  constructor(props) {
    super(props)
    this.state = {
      locationOfVehicle: {
        X: 0.0,
        Y: 0.0
      }
    }
  }

  componentWillUpdate = prevProps => {
    // console.log('did update', this.props.distanceX, this.props.distanceY)
    if (prevProps === this.props) {
      return
    }

    let location = this.computeLocationOfVehicle()
    this.setState({
      locationOfVehicle: location
    })
  }

  computeLocationOfVehicle = () => {
    const { distanceX, distanceY, angle } = this.props
    /* ------------------- Compute the Center of the vehicle-----------
    Xb = Xv - w/2*sin(angle)
    Yb = Yv + w/2*cos(angle)

    where Xv and Yv is distanceX,distanceY from telemetry data and w is the approximate meter-dimentions of the robot's width and height. */
    const robotDimentionsMeters = 0.2612
    let Xb = distanceX - robotDimentionsMeters * Math.sin(angle)
    let Yb = distanceY + robotDimentionsMeters * Math.cos(angle)
    /* ------------------- Compute the Location of the vehicle-----------
    X = Px/A * (Xb -B) + D
    Y = Px/A * (Yb -C) + E

    - where Px is dimentions in pixels of animtion container, (268px * 268px for now)
    - where A is dimentions in meters of robot container space (for now its 2m * 2m)
    - (Xb,Yb) is the center coordinate of the vehicle
    - (D,E) is the top left location of the div within the browser window
    */
    // + (dockingDivLocation[0]
    // + (dockingDivLocation[1]
    const pixelConversionOfContainer = 134
    return {
      X: pixelConversionOfContainer * (Xb - -1) - 35,
      Y: pixelConversionOfContainer * (Yb - -0.2) - 35
    }
  }

  render() {
    const {
      angle,
      distanceX,
      distanceY,
      status,
      setDockingDivLocation
    } = this.props
    const { locationOfVehicle } = this.state

    return (
      <Container>
        <TelemetryData
          distanceX={distanceX}
          distanceY={distanceY}
          angle={angle}
        />
        <DockingAnimation
          angle={angle}
          distanceX={distanceX}
          distanceY={distanceY}
          status={status}
          locationOfVehicle={locationOfVehicle}
          setDockingDivLocation={setDockingDivLocation}
        />
      </Container>
    )
  }
}

// map redux store to component props
const mapStateToProps = state => {
  return {
    dockingDivLocation: state.dockingDivLocation
  }
}

const mapDispatchToProps = dispatch => {
  return bindActionCreators({ ...vehicleDucks, push }, dispatch)
}

export default connect(mapStateToProps, mapDispatchToProps)(Docking)
