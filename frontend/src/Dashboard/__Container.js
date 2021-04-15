import React from 'react'
import PropTypes from 'prop-types'
import styled from 'styled-components'
import { connect } from 'react-redux'
import { bindActionCreators } from 'redux'
import { push } from 'react-router-redux'
import * as vehicleDucks from '../redux/modules/vehicles'
import * as notificationDucks from '../redux/modules/notifications'
import CommonShapes from '../Shared/CommonShapes'
import Navigation from './Navigation'
import Title from './Title'
import VehicleCard from './VehicleCard'
import AddSimulatorCard from './AddSimulatorCard'
import WebSocketClient from '../utils/WebSocket'

const Container = styled.div`
  background: #f5f7fa;
  min-height: 100vh;
`

const VehicleContainer = styled.div`
  display: flex;
  flex-flow: row wrap;
  margin-left: 82px;
  margin-top: 20px;
  transition: 0.2s all;
`

export class Dashboard extends React.Component {
  static propTypes = {
    addVehicle: PropTypes.func.isRequired,
    editVehicle: PropTypes.func.isRequired,
    fetchVehicles: PropTypes.func.isRequired,
    fetchAndUpdateVehicle: PropTypes.func.isRequired,
    killSimulator: PropTypes.func.isRequired,
    setVehicleStatus: PropTypes.func.isRequired,
    getLatestCameraFrame: PropTypes.func.isRequired,
    push: PropTypes.func.isRequired,
    requestControl: PropTypes.func.isRequired,
    addNotification: PropTypes.func.isRequired,
    vehicles: PropTypes.arrayOf(CommonShapes.Vehicle).isRequired
  }

  constructor(props) {
    super(props)

    this.ws = null
  }

  componentDidMount() {
    this.props.fetchVehicles()
  }

  componentWillMount() {
    if (this.ws) this.ws.closeSocket()
  }

  /**
   * Creates a placeholder vehicle until we get a "simulator up" message in the
   * notify socket
   */
  addPlaceholderVehicle = (
    id,
    status = vehicleDucks.VEHICLE_STATUS.LAUNCHING,
    cameras = []
  ) => {
    const { VEHICLE_TYPES } = vehicleDucks
    const vehicle = {
      id,
      status,
      vehicle_id: id,
      cameras,
      location: {
        location: {
          lat: 37.3382,
          lon: 121.8863,
          alt: 12345
        },
        timestamp: new Date().toUTCString()
      },
      operator: {
        name: ''
      },
      type: VEHICLE_TYPES.SIMULATOR
    }

    this.props.addVehicle(vehicle)
    this.openNotifyWS(id)
  }

  openNotifyWS = id => {
    this.ws = new WebSocketClient(
      this,
      `ws${window.zippyconfig.scheme}://${window.zippyconfig
        .backend}/api/v1/ws/simulator/${id}/notifier`,
      {
        name: 'SimulatorNotifier',
        onmessage: this.handleMessage
      }
    )
  }

  handleMessage = msg => {
    const message = JSON.parse(msg.data)

    // Other statuses could be 'start' and 'ping', neither of which we care
    // about at the moment
    if (message.status === 'ready') {
      this.props.fetchAndUpdateVehicle(message.id)
      this.ws.closeSocket()
    } else if (message.status === 'error') {
      this.props.setVehicleStatus(message.id, 'disabled')
      this.ws.closeSocket()
    }
  }

  handleClickOperate = (vehicleId, camera) => {
    const { push } = this.props

    push(`/app/vehicle/${vehicleId}/${camera}`)
  }

  handleClickShutdownSimulator = vehicleId => {
    const { killSimulator, push } = this.props
    killSimulator(vehicleId)
      .then(() => {
        push('/')
      })
      .catch(error => {
        // this.setState({ disabled: false })
      })
  }

  render() {
    const { vehicles, getLatestCameraFrame } = this.props
    const { getFrontCamera, VEHICLE_TYPES } = vehicleDucks

    return (
      <Container>
        <Navigation />
        <Title />
        <VehicleContainer>
          {vehicles.map(vehicle => {
            const frontCamera = getFrontCamera(vehicle)
            const role = frontCamera ? frontCamera.role : null

            return (
              <VehicleCard
                key={vehicle.id}
                vehicleId={vehicle.id}
                // Why is this called realVehicleId? Because currently we are
                // using sessionId in place of vehicleId everywhere. This works
                // well for the most part, but it's a stopgap until point and go
                // can actually understand vehicle_ids.
                realVehicleId={vehicle.vehicle_id}
                cameraRole={role}
                status={vehicle.status}
                type={vehicle.type ? vehicle.type : VEHICLE_TYPES.VEHICLE}
                getLatestCameraFrame={getLatestCameraFrame}
                handleClickOperate={this.handleClickOperate}
                handleClickShutdown={this.handleClickShutdownSimulator}
              />
            )
          })}
          <AddSimulatorCard onComplete={this.addPlaceholderVehicle} />
        </VehicleContainer>
      </Container>
    )
  }
}

const mapStateToProps = state => {
  return {
    vehicles: state.vehicles.vehicles
  }
}

const mapDispatchToProps = dispatch => {
  return bindActionCreators(
    { ...vehicleDucks, ...notificationDucks, push },
    dispatch
  )
}

export default connect(mapStateToProps, mapDispatchToProps)(Dashboard)
