import React from 'react'
import PropTypes from 'prop-types'
import { connect } from 'react-redux'
// import components
import RobotIllustration from './RobotIllustration.js'
import DockIllustration from './DockIllustration.js'

class DockingAnimation extends React.Component {
  static propTypes = {
    dispatch: PropTypes.func.isRequired,
    angle: PropTypes.number.isRequired,
    locationOfVehicle: PropTypes.object.isRequired,
    // locationOfVehicle: PropTypes.arrayOf(PropTypes.number).isRequired,
    setDockingDivLocation: PropTypes.func.isRequired,
    status: PropTypes.string
  }

  static defaultProps = {
    status: ''
  }

  constructor(props) {
    super(props)
    this.state = {
      location: [0, 0]
    }
  }
  componentDidMount = () => {
    this.setState({ location: this.getRectLocation() })
  }

  componentDidUpdate = () => {
    this.dispatchLocation(this.state.location)
  }

  getRectLocation = () => {
    const boundingRect = this.dock.getBoundingClientRect()
    return [boundingRect.left, boundingRect.top]
  }

  dispatchLocation = location => {
    // dispatch an action to set the Div's location to the redux store
    this.props.setDockingDivLocation(location)
  }

  render(props) {
    const { angle, status, locationOfVehicle } = this.props
    return (
      <div
        className="animation-container"
        ref={dock => {
          this.dock = dock
        }}
      >
        <DockIllustration />
        <RobotIllustration location={locationOfVehicle} angle={angle} />
        <div className="status-message">
          {status === 'INPROGRESS' ? 'In Progress' : 'Stopped'}
        </div>
      </div>
    )
  }
}

// map redux store to component props
const mapStateToProps = state => {
  return {
    location: state.location
  }
}
export default connect(mapStateToProps, null)(DockingAnimation)
