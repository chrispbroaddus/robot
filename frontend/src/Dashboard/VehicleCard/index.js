import React from 'react'
import PropTypes from 'prop-types'
import './vehicle-card.styles.scss'
import DefaultImage from '../../images/default.png'
import CardTitle from './CardTitle'
import ImageStatus from './ImageStatus'
import DashboardButton from '../DashboardButton'
import { VEHICLE_STATUS, VEHICLE_TYPES } from '../../redux/modules/vehicles'

class VehicleCard extends React.Component {
  static propTypes = {
    vehicleId: PropTypes.string.isRequired,
    handleClickOperate: PropTypes.func.isRequired,
    handleClickShutdown: PropTypes.func.isRequired,
    cameraRole: PropTypes.string,
    status: PropTypes.string,
    type: PropTypes.string,
    realVehicleId: PropTypes.string.isRequired,
    getLatestCameraFrame: PropTypes.func.isRequired
  }

  static defaultProps = {
    cameraRole: '',
    status: '',
    type: ''
  }

  constructor(props) {
    super(props)

    this.state = {
      imageUrl: null
    }
  }

  componentWillMount() {
    const { getLatestCameraFrame, realVehicleId } = this.props
    getLatestCameraFrame(realVehicleId, 'FrontFisheye')
      .then(data => {
        this.setState({
          imageUrl: data.url
        })
      })
      .catch(() => {
        this.setState({
          imageUrl: DefaultImage
        })
      })
  }

  render() {
    const {
      vehicleId,
      cameraRole,
      status,
      type,
      handleClickOperate,
      handleClickShutdown
    } = this.props
    let { imageUrl } = this.state
    imageUrl = imageUrl ? imageUrl : DefaultImage

    return (
      <div className="card">
        <ImageStatus src={imageUrl} status={status} title={vehicleId} />
        <CardTitle title={vehicleId} />
        <div className="center">
          {status === VEHICLE_STATUS.NETWORK_DROP && (
            <p className="wait-text">
              Connection difficulties. Try refreshing.
            </p>
          )}
          {status === VEHICLE_STATUS.LAUNCHING && (
            <p className="wait-text">Launching Simulator . . .</p>
          )}
          {status === VEHICLE_STATUS.ACTIVE && (
            <DashboardButton
              buttonClassName={status}
              disabled={status === VEHICLE_STATUS.DISABLED}
              onClick={() => handleClickOperate(vehicleId, cameraRole)}
            >
              operate
            </DashboardButton>
          )}
          {type === VEHICLE_TYPES.SIMULATOR && (
            <DashboardButton
              buttonClassName={status}
              disabled={status === VEHICLE_STATUS.DISABLED}
              onClick={() => handleClickShutdown(vehicleId)}
            >
              shut down
            </DashboardButton>
          )}
        </div>
      </div>
    )
  }
}

export default VehicleCard
