import React from 'react'
import PropTypes from 'prop-types'
import styled from 'styled-components'
import { bindActionCreators } from 'redux'
import { push } from 'react-router-redux'
import { connect } from 'react-redux'
import { v4 as uuid } from 'uuid'
import './camera-view.syles.scss'
import Logo from '../Shared/Logo'
import CommandText from './CommandText'
import MapComponent from './Map'
import DashboardNavbar from './DashboardNavbar'
import CameraViewButton from './CameraViewButton'
import ArcControl from './ArcControl'
import WebRTCStream from './WebRTCStream'
import DockingUIContainer from './DockingUI/__Container'
import CommandView from './CommandView'
import Help from './Help'
import LeftControls from './LeftControls'
import NotificationBanner from './NotificationBanner'
import ReverseDisplay from './ReverseDisplay'
import ReverseFrame from './ReverseFrame'
import Tint from './Tint'
import Overlay from './Overlay'
import CommandOverrideMessage from './CommandOverrideMessage'
import TurnInPlaceMenu from './TurnInPlaceMenu'
import ExposureRing from './ExposureRing'
import ExposureMessage from './ExposureMessage'
import WebGL from './WebGL'
import CommonShapes from '../Shared/CommonShapes'
import WebSocketClient from '../utils/WebSocket'
import ZStage from './ZStage/__Container'
import { manifestFactory } from '../utils/manifest'
import * as trans from '../utils/translations'
import * as commandDucks from '../redux/modules/commands'
import * as vehicleDucks from '../redux/modules/vehicles'
import * as notificationDucks from '../redux/modules/notifications'

// Taken from https://github.com/davidcalhoun/gps-time.js/blob/master/gps-time.js#L39
const gpsUnixEpochDiffMS = 315964800000

const Container = styled.div`
  position: relative;
  display: flex;
  flex-flow: column nowrap;
  width: 100%;
  height: 100%;
  overflow: hidden;
`

const InnerContainer = styled.div`
  flex: 1 1 auto;
  position: relative;
  padding-top: 14px;

  & svg[name='exposure-ring'] {
    position: absolute;
    top: 0;
    left: 0;
    z-index: 750;
    opacity: 0;
    transition: 0.2s opacity;
  }
`

const StreamContainer = styled.div`
  width: 100%;
  height: 100%;
  position: absolute;
  top: 0;
  left: 0;
  z-index: 80;
  user-select: none;
`

const LogoContainer = styled(Logo)`
  margin-left: 39px;
  position: relative;
  z-index: 1000;
`

// Enumerables to help us stay consistent throughout the app (instead of
// relying on string values)
export const COMMAND_MODES = {
  PAG: 'pag',
  ARC: 'arc'
}
export const EXPOSURE_MODES = {
  AUTO: 'auto',
  RING: 'ring'
}
export const MODIFIER_KEYS = {
  ALT: 'altKey',
  CTRL: 'ctrlKey',
  META: 'metaKey',
  SHIFT: 'shiftKey'
}

// Change this to change the onClick modifier key for the exposure
export const EXPOSURE_MODIFIER = MODIFIER_KEYS.SHIFT
export const EXPOSURE_RADIUS = 75
export const EXPOSURE_TIMEOUT = 3500

export const WEBSOCKET_BASE_URL = `ws${window.zippyconfig.scheme}://${window
  .zippyconfig.backend}/api/v1/ws/vehicle`

class CameraView extends React.Component {
  static propTypes = {
    match: PropTypes.shape({
      params: PropTypes.shape({
        vehicleId: PropTypes.string.isRequired,
        cameraRole: PropTypes.string.isRequired
      }).isRequired
    }).isRequired,
    history: PropTypes.shape({
      push: PropTypes.func.isRequired
    }).isRequired,
    relinquishVehicle: PropTypes.func.isRequired,
    requestControl: PropTypes.func.isRequired,
    addNotification: PropTypes.func.isRequired,
    push: PropTypes.func.isRequired,
    addCommand: PropTypes.func.isRequired,
    setCommandStatus: PropTypes.func.isRequired,
    getCameraIdByRole: PropTypes.func.isRequired,
    fetchAndUpdateVehicle: PropTypes.func.isRequired,
    setCurrentVehicle: PropTypes.func.isRequired,
    // From mapStateToProps
    currentVehicle: CommonShapes.Vehicle,
    currentCamera: CommonShapes.Camera,
    vehicleId: PropTypes.string.isRequired,
    cameraRole: PropTypes.string.isRequired
  }

  constructor(props) {
    super(props)
    this.state = {
      stationIDs: [],
      dockingStation: 0,
      status: '',
      distanceX: 0.0,
      distanceY: 0.0,
      angle: 0.0,
      dockingSuccess: false,
      dockingFailed: false,
      stopped: false,
      toggleBanner: false,
      dockingModeActivated: false,
      exposureMode: EXPOSURE_MODES.AUTO,
      showDistortionGrid: false,
      showGroundPlane: false,
      showBoundingBoxes: true,
      show3dBoundingBoxes: false,
      showConvexHulls: false,
      manifest: manifestFactory(null),
      helpModalIsOpen: false,
      zStageIsOpen: false,
      isCommandOverrideMode: false,
      commandMode: COMMAND_MODES.ARC,
      boundingBoxes: [],
      boundingBoxes3d: [],
      convexHulls: []
    }

    this.stream = null
  }

  componentWillMount() {
    const {
      vehicleId,
      fetchAndUpdateVehicle,
      setCurrentVehicle,
      addNotification
    } = this.props

    fetchAndUpdateVehicle(vehicleId)
      .then(() => {
        setCurrentVehicle(vehicleId)
      })
      .catch(err => {
        addNotification(err, notificationDucks.NOTIFICATION_STATUS.DANGER, 0)
        // Request control failed, navigate back to the dashboard
        push('/app/dashboard')
      })
  }

  componentWillReceiveProps(nextProps) {
    const { currentCamera } = this.props
    const { manifest } = this.state

    // If a new camera emerges (checking if we're going from null to something
    // or a camera ID change), load its manifest into the state
    if (
      (currentCamera === null && nextProps.currentCamera !== null) ||
      (currentCamera && currentCamera.id !== nextProps.currentCamera.id) ||
      // This last condition will make sure we're loading the manifest when
      // going from dashboard to CameraView
      (nextProps.currentCamera &&
        manifest.cameraId !== nextProps.currentCamera.id)
    ) {
      this.setState({
        manifest: manifestFactory(nextProps.currentCamera)
      })
    }
  }

  componentDidMount() {
    const {
      vehicleId,
      cameraRole,
      requestControl,
      push,
      addNotification
    } = this.props

    // Add an event listener for key down
    if (window) window.addEventListener('keypress', this.handleKeyDown)
    if (window) window.addEventListener('resize', this.handleResize)

    // Request control and then connect to the websocket
    requestControl(vehicleId)
      .then(() => {
        const viewWSUrl = `${WEBSOCKET_BASE_URL}/${vehicleId}/view`
        const commandWSUrl = `${WEBSOCKET_BASE_URL}/${vehicleId}/command`

        this.ws = new WebSocketClient(this, commandWSUrl, {
          name: 'CameraView_Command',
          onmessage: this.handleCommandMessage
        })
        this.viewWs = new WebSocketClient(this, viewWSUrl, {
          name: 'CameraView_View',
          onmessage: this.handleViewMessage
        })
        this.emptyStationChecker()
      })
      .catch(() => {
        addNotification(
          'Could not request control of vehicle',
          notificationDucks.NOTIFICATION_STATUS.DANGER
        )
        // Request control failed, navigate back to the dashboard
        push('/app/dashboard')
      })

    // Display the reverse command text if we've been mounted in rear camera
    // mode
    if (cameraRole === vehicleDucks.CAMERA_ROLES.RearFisheye) {
      this.setCommandTextReverseMode()
    }
  }

  componentWillUnmount() {
    if (this.ws) this.ws.closeSocket()
    if (this.viewWs) this.viewWs.closeSocket()

    // Remove event listener for command switcher
    if (window) {
      window.removeEventListener('keypress', this.handleKeyDown)
    }
  }

  handleResize = () => {
    // Debounce window resize events to 50 ms
    if (this.timeout) {
      clearTimeout(this.timeout)
    }
    this.timeout = setTimeout(() => {
      this.resize()
      this.timeout = null
    }, 50)
  }

  resize = () => {
    // If the video stream is active, get its dimensions and save it to the
    // state to eventually propagate it down to the children
    if (!this.stream || this.stream.getBoundingClientRect === undefined) {
      this.setState(
        {
          videoTop: 0,
          videoLeft: 0,
          videoWidth: window.innerWidth,
          videoHeight: window.innerHeight
        },
        () => {
          this.forceUpdate()
        }
      )
    } else {
      const streamRect = this.stream.getBoundingClientRect()
      const { top, left, width, height } = streamRect
      this.setState(
        {
          videoTop: top,
          videoLeft: left,
          videoWidth: width,
          videoHeight: height
        },
        () => {
          this.forceUpdate()
        }
      )
    }
  }

  relinquish = () => {
    const { addNotification, vehicleId, relinquishVehicle, push } = this.props

    relinquishVehicle(vehicleId)
      .then(response => {
        // Return to the dashboard
        push('/app/dashboard')
      })
      .catch(error => {
        addNotification(
          `Error: ${error}`,
          notificationDucks.NOTIFICATION_STATUS.DANGER
        )
      })
  }

  emptyStationChecker() {
    // Check if stations are empty
    if (this.state.stationIDs.length === 0) {
      this.setState({ toggleBanner: false })
    }
  }

  /**
   * Handles sending the Z-Stage message to the backend
   * @param  {Float}  zStageValue Z-Stage value
   */
  handleSendZStage = zStageValue => {
    if (!this.ws) return // WebSocket isn't ready

    const msg = {
      zStage: {
        position: parseFloat(zStageValue)
      },
      id: uuid()
    }
    this.ws.send(JSON.stringify(msg))
    this.props.addCommand(
      msg.id,
      commandDucks.COMMAND_TYPE.zStage,
      commandDucks.COMMAND_STATUS.sent
    )
  }

  handleArcClick = (x, y, angle) => {
    if (!this.ws) return // WebSocket isn't ready

    const { cameraRole, currentVehicle } = this.props
    const { isCommandOverrideMode } = this.state

    const { id: cameraId } = vehicleDucks.getCameraByRole(
      currentVehicle,
      cameraRole
    )
    console.log(`sending arc command: ${x}, ${y} @ ${angle}°`)
    let now = {
      nanos: (Date.now() - gpsUnixEpochDiffMS) * 1000000
    }

    const msg = {
      pointAndGo: {
        imageX: x,
        imageY: y,
        camera: cameraId,
        imageTimestamp: now,
        operatorTimestamp: now,
        commandOverrideFlag: isCommandOverrideMode
      },
      id: uuid()
    }
    this.ws.send(JSON.stringify(msg))
    this.props.addCommand(
      msg.id,
      commandDucks.COMMAND_TYPE.pointAndGo,
      commandDucks.COMMAND_STATUS.sent
    )
  }

  handlePointAndGo = ev => {
    if (!this.ws) return // WebSocket isn't ready

    const { cameraRole, currentVehicle } = this.props
    const { isCommandOverrideMode } = this.state

    const { id: cameraId } = vehicleDucks.getCameraByRole(
      currentVehicle,
      cameraRole
    )

    // this function call is to access the event asynchronously
    // ev.persist()

    // Don't try to access the stream functions if it's not set up yet
    if (!this.stream || !this.stream.getBoundingClientRect) {
      return
    }

    // get the mouse and video position
    let rect = this.stream.getBoundingClientRect()

    let x = ev.clientX - rect.left
    let y = ev.clientY - rect.top

    let videoWidth = this.stream.getVideoWidth()
    let videoHeight = this.stream.getVideoHeight()

    // compute the origin
    let offsetX = 0
    let offsetY = 0
    let scale = 0
    if (rect.width * videoHeight < rect.height * videoWidth) {
      // video is wider than rect
      scale = rect.width / videoWidth
      offsetY = (rect.height - videoHeight * scale) / 2
    } else {
      // video is taller than rect
      scale = rect.height / videoHeight
      offsetX = (rect.width - videoWidth * scale) / 2
    }

    const xx = (x - offsetX) / (scale * videoWidth)
    const yy = (y - offsetY) / (scale * videoHeight)

    if (xx < 0 || xx > 1 || yy < 0 || yy > 1) {
      console.log(
        'ignoring point and go command out of range: ',
        x,
        y,
        rect,
        offsetX,
        offsetY
      )
      return
    }

    const now = {
      nanos: (gpsUnixEpochDiffMS + Date.now()) * 1000000
    }

    // TODO: this function will be required to be throttled
    const msg = {
      pointAndGo: {
        imageX: xx,
        imageY: yy,
        camera: cameraId,
        imageTimestamp: now, // TODO: use current image timestamp from vehicle
        operatorTimestamp: now,
        commandOverrideFlag: isCommandOverrideMode
      },
      id: uuid()
    }
    this.ws.send(JSON.stringify(msg))
    this.props.addCommand(
      msg.id,
      commandDucks.COMMAND_TYPE.pointAndGo,
      commandDucks.COMMAND_STATUS.sent
    )
  }

  handleCommandMessage = msg => {
    const payload = JSON.parse(msg.data)
    if (payload.dockingObservation) {
      if (payload.dockingObservation.stationIds) {
        this.setState({
          stationIDs: payload.dockingObservation.stationIds,
          toggleBanner: true
        })
      }
    } else if (payload.dockingStatus) {
      if (payload.dockingStatus.status === 'SUCCESS') {
        this.setState({
          dockingSuccess: true,
          dockingFailed: false,
          dockingStation: 0
        })
      } else if (payload.dockingStatus.status === 'FAILURE') {
        this.setState({
          dockingFailed: true,
          dockingSuccess: false,
          dockingStation: 0
        })
      } else {
        this.setState({
          status: payload.dockingStatus.status,
          distanceX: payload.dockingStatus.remainingDistanceX,
          distanceY: payload.dockingStatus.remainingDistanceY,
          angle: payload.dockingStatus.remainingAngle,
          dockingFailed: false,
          dockingSuccess: false
        })
      }
    } else if (payload.confirmation) {
      const { setCommandStatus } = this.props
      // From Go: messageId is the commandId.

      // Ignore if the command doesn't have a commandId or is a failure (this is
      // temporary)
      if (
        payload.confirmation.messageId &&
        !payload.confirmation.status &&
        payload.confirmation.status !== 'FAILURE'
      )
        setCommandStatus(
          payload.confirmation.messageId,
          commandDucks.COMMAND_STATUS.confirmed
        )
    }
  }

  handleViewMessage = msg => {
    const payload = JSON.parse(msg.data)
    if (
      payload.detection &&
      this.state.showBoundingBoxes == true &&
      Array.isArray(payload.detection.boundingBoxes)
    ) {
      this.setState({
        // HACK: We are filtering for certain categories temporarily until we
        // fine-tune which combination of bounding boxes is optimal for the user
        boundingBoxes: payload.detection.boundingBoxes.filter(
          b =>
            (b.category && !b.category.type) ||
            (b.category.type &&
              (b.category.type === 'UNKNOWN' || b.category.type === 'ANIMAL'))
        )
      })
    } else if (payload.detection3d) {
      if (
        this.state.show3dBoundingBoxes == true &&
        Array.isArray(payload.detection3d.boundingBoxes)
      ) {
        this.setState({
          // HACK: We are filtering for certain categories temporarily until we
          // fine-tune which combination of bounding boxes is optimal for the user
          boundingBoxes3d: payload.detection3d.boundingBoxes.filter(
            b => b.category && b.category.type && b.category.type === 'PERSON'
          )
        })
      }
      if (
        this.state.showConvexHulls == true &&
        Array.isArray(payload.detection3d.convexHulls)
      ) {
        this.setState({
          // HACK: We are filtering for certain categories temporarily until we
          // fine-tune which combination of bounding boxes is optimal for the user
          convexHulls: payload.detection3d.convexHulls.filter(
            b =>
              b.category &&
              b.category.type &&
              (b.category.type === 'CAR' ||
                b.category.type === 'TRUCK' ||
                b.category.type === 'BUS' ||
                b.category.type === 'MOTORBIKE' ||
                b.category.type === 'BICYCLE')
          )
        })
      }
    }
  }

  // temp until we add real implementation for docking
  callDock = station => {
    const msg = {
      dockCommand: {
        stationId: station
      },
      id: uuid()
    }
    this.ws.send(JSON.stringify(msg))
    this.props.addCommand(
      msg.id,
      commandDucks.COMMAND_TYPE.dockCommand,
      commandDucks.COMMAND_STATUS.sent
    )
    this.setState(prevState => ({
      dockingStation: station,
      dockingModeActivated: !prevState.dockingModeActivated
    }))

    this.setCommandText('Docking Mode is', 'Activated')
  }

  /**
   * Sends the STOP command to the vehicle
   */
  stopVehicle = () => {
    const { addCommand } = this.props
    const msg = {
      stopCommand: {},
      id: uuid()
    }
    this.ws.send(JSON.stringify(msg))
    addCommand(
      msg.id,
      commandDucks.COMMAND_TYPE.stopVehicle,
      commandDucks.COMMAND_STATUS.sent
    )
  }

  /**
   * Handles all keybindings on this component
   * @param  {Event} event KeyPress event
   */
  handleKeyDown = event => {
    const { currentCamera: { id: cameraId } } = this.props
    const key = event.key.toLowerCase()

    if (event.keyCode === 63) {
      // Forward slash
      this.toggleHelpModal()
    } else if (key === 'r') {
      this.switchCamera()
    } else if (key === 'o') {
      this.setState(prevState => ({
        showDistortionGrid: !prevState.showDistortionGrid
      }))
    } else if (key === 'l') {
      this.setState(prevState => ({
        showGroundPlane: !prevState.showGroundPlane
      }))
    } else if (key === 'b') {
      this.setState(prevState => ({
        showBoundingBoxes: !prevState.showBoundingBoxes
      }))
    } else if (key === '3') {
      this.setState(prevState => ({
        show3dBoundingBoxes: !prevState.show3dBoundingBoxes
      }))
    } else if (key === '4') {
      this.setState(prevState => ({
        showConvexHulls: !prevState.showConvexHulls
      }))
    } else if (key === 's') {
      this.stopVehicle()
    } else if (key === 'e') {
      this.sendResetExposure(cameraId)
    } else if (event.keyCode === 27) {
      // Close modals and context menus when escape is pressed
      this.setState({
        contextMenuIsOpen: false,
        helpModalIsOpen: false
      })
      console.log('escape menu is hit')
    } else if (key === 'z') {
      this.toggleZStage()
    } else if (key === 'c') {
      this.switchCommandOverrideMode()
    }
  }

  toggleBanner = () => {
    this.setState(prevState => ({ toggleBanner: !prevState.toggleBanner }))
  }

  /**
   * Toggle between command override mode
   * @param  {String} inputMode   If an inputMode is explicitly specified,
   *                              don't toggle and switch to that mode
   *                              instead.
   */
  switchCommandOverrideMode = (inputMode = null) => {
    const { isCommandOverrideMode } = this.state
    let mode
    if (inputMode) {
      mode = inputMode
    } else {
      mode = !isCommandOverrideMode
    }

    this.setState({
      isCommandOverrideMode: mode
    })
  }

  /**
   * Toggle between command modes (Arc or Point and Go)
   * @param  {String} inputMode   If an inputMode is explicitly specified,
   *                              don't toggle and switch to that mode
   *                              instead. Expects either 'arc' or 'pag' at
   *                              the moment.
   */
  switchCommandMode = (inputMode = null) => {
    let mode
    if (inputMode) {
      mode = inputMode
    } else {
      mode =
        inputMode === COMMAND_MODES.PAG ? COMMAND_MODES.ARC : COMMAND_MODES.PAG
    }

    if (mode !== COMMAND_MODES.PAG && mode !== COMMAND_MODES.ARC) {
      // Subject to change as we include other cameras
      throw new Error('Expected command mode to be "arc" or "pag"')
    }

    this.setState({
      commandMode: mode
    })
  }

  /**
   * Toggle between exposure modes
   * @param  {String} inputMode   If an inputMode is explicitly specified,
   *                              don't toggle and switch to that mode
   *                              instead. Expects either 'auto' or 'ring'.
   */
  switchExposureMode = (inputMode = null) => {
    const { cameraRole, currentVehicle } = this.props
    const { id: cameraId } = vehicleDucks.getCameraByRole(
      currentVehicle,
      cameraRole
    )

    let mode
    if (inputMode) {
      mode = inputMode
    } else {
      mode =
        inputMode === EXPOSURE_MODES.AUTO
          ? EXPOSURE_MODES.RING
          : EXPOSURE_MODES.AUTO
    }

    if (mode !== EXPOSURE_MODES.AUTO && mode !== EXPOSURE_MODES.RING) {
      // Subject to change as we include other cameras
      throw new Error('Expected exposure mode to be "ring" or "auto"')
    }

    // If we're changing to AUTO, reset the last x/y of the exposure point and
    // hide the active exposure ring
    if (mode === EXPOSURE_MODES.AUTO) {
      this.sendResetExposure(cameraId)
    } else {
      this.setState({
        exposureMode: mode
      })
    }
  }

  /**
   * Toggle between the front & reverse camera
   * @param  {String} inCameraRole  If an inCameraRole is explicitly specified,
   *                                don't flip the camera but switch to that
   *                                camera role instead.
   */
  switchCamera = (inCameraRole = null) => {
    const { vehicleId, cameraRole, currentVehicle, currentCamera } = this.props
    const { flipCamera, isRearCamera } = vehicleDucks
    let cameraRoleParam
    if (inCameraRole) {
      // If a camera was specified, leave as-is
      cameraRoleParam = inCameraRole
    } else {
      // Flip the camera
      const cameras = currentVehicle.cameras
      cameraRoleParam = flipCamera(cameras, cameraRole)
    }

    // Switch cameras
    this.props.push(`/app/vehicle/${vehicleId}/${cameraRoleParam}`)

    // Do certain things if we're on the rear camera
    if (isRearCamera(cameraRoleParam)) {
      this.setCommandTextReverseMode()
    } else {
      // Clears the command text
      this.setCommandText()
    }

    // Resets the exposure ring
    this.sendResetExposure(currentCamera.id)
  }

  setCommandTextReverseMode = () => {
    this.setCommandText('Reverse Mode', 'Activated', '')
  }

  setCommandText = (main = '', command = '', rest = '') => {
    // If everything is blank, send in a null so the CommandText component will
    // hide itself
    if (main === '' && command === '' && rest === '') {
      this.setState({
        commandMessage: null
      })
    } else {
      this.setState({
        commandMessage: {
          main,
          command,
          rest
        }
      })
    }
  }

  handleMouseMove = evt => {
    let {
      videoTop: top,
      videoLeft: left,
      videoWidth: width,
      videoHeight: height
    } = this.state

    top = top ? top : 0
    left = left ? left : 0
    width = width ? width : window.innerWidth
    height = height ? height : window.innerHeight

    const mouseX = evt.nativeEvent.clientX
    const mouseY = evt.nativeEvent.clientY
    const exposureModifierPressed = evt.nativeEvent[EXPOSURE_MODIFIER]

    // Sets boundaries for when the exposure ring will be drawn (within the
    // frame of the video)
    const isInBoundsX = mouseX - left > 0 && mouseX < left + width
    const isInBoundsY = mouseY - top > 0 && mouseY < top + height
    if (this.mouseOverExpRing && this.activeExpRing) {
      if (exposureModifierPressed === true && isInBoundsY && isInBoundsX) {
        const box = this.mouseOverExpRing.getBoundingClientRect()

        this.mouseOverExpRing.style.opacity = 1
        this.mouseOverExpRing.style.transform = `translate(${mouseX -
          box.width / 2}px, ${mouseY - box.height / 2}px)`
        this.mouseOverExpRing.style.zIndex = 1000
      } else {
        this.mouseOverExpRing.style.opacity = 0
        this.mouseOverExpRing.style.zIndex = 0
      }
    }
    // Don't do anything if the ref isn't tied to the instance variable yet
    // console.log('moving', evt.nativeEvent.clientX, evt.nativeEvent.clientY)
  }

  handleClick = evt => {
    // console.log('clicking', evt.nativeEvent.clientX, evt.nativeEvent.clientY)
  }

  /**
   * Displays the active exposure ring
   * @param  {Number} x x position in pixels
   * @param  {Number} y y position in pixels
   */
  displayExposureRing = (x, y) => {
    this.activeExpRing.style.opacity = 1
    this.activeExpRing.style.transform = `translate(${x}px, ${y}px)`
    this.activeExpRing.style.zIndex = 400
  }

  /**
   * Sends the reset exposure command over the WebSocket
   * @param  {String} cameraId Camera ID
   */
  sendResetExposure = cameraId => {
    this.hideExposureRing()
    this.resetExposureRing()

    const msg = {
      resetExposure: {
        camera_id: cameraId
      },
      id: uuid()
    }
    this.ws.send(JSON.stringify(msg))
    this.props.addCommand(
      msg.id,
      commandDucks.COMMAND_TYPE.resetExposure,
      commandDucks.COMMAND_STATUS.sent
    )
  }

  /**
   * Hides the exposure ring
   */
  hideExposureRing = () => {
    this.activeExpRing.style.opacity = 0
    this.activeExpRing.style.zIndex = 0
  }

  /**
   * Reset exposure ring
   */
  resetExposureRing = () => {
    this.hideExposureRing()
    this.setState({
      exposureMode: EXPOSURE_MODES.AUTO,
      lastExposureX: null,
      lastExposureY: null
    })
  }

  /**
   * Handles the click even when the exposure modifier is pressed
   * @param  {Event} evt Click event
   */
  handleClickExposure = evt => {
    const { cameraRole, currentVehicle } = this.props
    const { manifest } = this.state
    const { id: cameraId } = vehicleDucks.getCameraByRole(
      currentVehicle,
      cameraRole
    )

    // Don't try to access the stream functions if it's not set up yet
    if (!this.stream || !this.stream.getBoundingClientRect) {
      return
    }

    // Get the mouse and video position
    let rect = this.stream.getBoundingClientRect()
    let x = evt.clientX - rect.left
    let y = evt.clientY - rect.top

    const { videoWidth, videoHeight } = this.state

    // Compute the origin
    let offsetX = 0
    let offsetY = 0
    let scale = 0
    if (rect.width * videoHeight < rect.height * videoWidth) {
      // Video is wider than rect
      scale = rect.width / videoWidth
      offsetY = (rect.height - videoHeight * scale) / 2
    } else {
      // Video is taller than rect
      scale = rect.height / videoHeight
      offsetX = (rect.width - videoWidth * scale) / 2
    }

    const xx = (x - offsetX) / (scale * videoWidth)
    const yy = (y - offsetY) / (scale * videoHeight)

    if (xx < 0 || xx > 1 || yy < 0 || yy > 1) {
      console.log('Ignoring out of range exposure: ', x, y)
      return
    }

    // Find the image coordinate of the LEFT side of the exposure circle
    const leftCircle = trans.scaleCanvasToImage(
      {
        x: Math.max(x - EXPOSURE_RADIUS, 0),
        y: y
      },
      manifest.intrinsics.resolution.x,
      manifest.intrinsics.resolution.y,
      this.state.videoWidth,
      this.state.videoHeight
    )
    // Find the image coordinate of the RIGHT side of the exposure circle
    const rightCircle = trans.scaleCanvasToImage(
      {
        x: Math.min(x + EXPOSURE_RADIUS, videoWidth),
        y: y
      },
      manifest.intrinsics.resolution.x,
      manifest.intrinsics.resolution.y,
      this.state.videoWidth,
      this.state.videoHeight
    )

    const percent = trans.getPercentageFromRange(
      {
        x: rightCircle.x - leftCircle.x,
        y: 0
      },
      { x: 0, y: 0 },
      {
        x: manifest.intrinsics.resolution.x,
        y: manifest.intrinsics.resolution.y
      }
    )

    // Radius in image space
    const imageRadius = percent.x.toFixed(3) / 100

    // Send the exposure command through the command WS channel
    const msg = {
      exposure: {
        camera_id: cameraId,
        centerX: xx,
        centerY: yy,
        radius: imageRadius
      },
      id: uuid()
    }
    this.ws.send(JSON.stringify(msg))
    this.props.addCommand(
      msg.id,
      commandDucks.COMMAND_TYPE.exposure,
      commandDucks.COMMAND_STATUS.sent
    )

    // Draw the ring
    const box = this.mouseOverExpRing.getBoundingClientRect()
    this.displayExposureRing(
      evt.clientX - box.width / 2,
      evt.clientY - box.height / 2
    )
    this.setState({
      exposureMode: EXPOSURE_MODES.RING
    })

    // Timeout the exposure
    this.exposureTimeout = setTimeout(() => {
      if (this.activeExpRing) {
        this.activeExpRing.style.opacity = 0
        this.activeExpRing.style.zIndex = 0
      }
    }, EXPOSURE_TIMEOUT)
  }

  openHelpModal = () => {
    this.toggleHelpModal(null, true)
  }

  closeHelpModal = () => {
    this.toggleHelpModal(null, false)
  }

  /**
   * Toggles the help modal's display
   * @param  {Event}   evt   Mouse event (ignored)
   * @param  {Boolean} input An explicit value for the open state
   */
  toggleHelpModal = (evt, input = null) => {
    if (input !== null && !(input instanceof Event)) {
      // Argument passed in, so not a toggle
      this.setState({ helpModalIsOpen: input })
    } else {
      // No arg, we should toggle
      this.setState(state => ({ helpModalIsOpen: !state.helpModalIsOpen }))
    }
  }

  handleContextMenu = evt => {
    evt.preventDefault()
    this.setState({
      contextMenuIsOpen: true,
      contextMenuTop: evt.clientY,
      contextMenuLeft: evt.clientX
    })
    return false
  }

  handleClick = evt => {
    this.toggleTurnInPlaceContextMenu(null, false)
  }

  sendTurnInPlaceCommand = (top, left) => {
    // TODO: Add real command through the WebSocket here
    console.log('sending turn in place', top, left)

    // Send the exposure command through the command WS channel
    const msg = {
      turnInPlace: {
        angleToTurnInRadians: Math.PI
      },
      id: uuid()
    }
    this.ws.send(JSON.stringify(msg))
    this.props.addCommand(
      msg.id,
      commandDucks.COMMAND_TYPE.turnInPlace,
      commandDucks.COMMAND_STATUS.sent
    )

    // Close the context menu after a successful send
    this.toggleTurnInPlaceContextMenu(false)
  }

  openZStage = () => {
    this.toggleZStage(null, true)
  }

  closeZStage = () => {
    this.toggleZStage(null, false)
  }

  /**
   * Toggles the z-stage display
   * @param  {Event}   evt   Mouse event (ignored)
   * @param  {Boolean} input An explicit value for the open state
   */
  toggleZStage = (evt, input = null) => {
    if (input !== null && !(input instanceof Event)) {
      // Argument passed in, so not a toggle
      this.setState({ zStageIsOpen: input })
    } else {
      // No arg, we should toggle
      this.setState(state => ({ zStageIsOpen: !state.zStageIsOpen }))
    }
  }

  /**
   * Toggles the turn in place menu
   * @param  {Event}   evt   Mouse event (ignored)
   * @param  {Boolean} input An explicit value for the open state
   */
  toggleTurnInPlaceContextMenu = (evt, input = null) => {
    if (input !== null && !(input instanceof Event)) {
      // Argument passed in, so not a toggle
      this.setState({ contextMenuIsOpen: input })
    } else {
      // No arg, we should toggle
      this.setState(state => ({ contextMenuIsOpen: !state.contextMenuIsOpen }))
    }
  }

  render() {
    const {
      stationIDs,
      dockingStation,
      status,
      distanceX,
      distanceY,
      angle,
      dockingFailed,
      dockingSuccess,
      stopped,
      toggleBanner,
      dockingModeActivated,
      commandMessage,
      videoTop,
      videoLeft,
      videoWidth,
      videoHeight,
      showDistortionGrid,
      showGroundPlane,
      showBoundingBoxes,
      show3dBoundingBoxes,
      showConvexHulls,
      exposureMode,
      manifest,
      helpModalIsOpen,
      isCommandOverrideMode,
      contextMenuIsOpen,
      contextMenuTop,
      contextMenuLeft,
      zStageIsOpen,
      boundingBoxes,
      boundingBoxes3d,
      convexHulls
    } = this.state

    const { cameraRole, addCommand, vehicleId, currentVehicle } = this.props
    const { isRearCamera, getCameraByRole } = vehicleDucks

    const isRear = isRearCamera(cameraRole)
    const camera = getCameraByRole(currentVehicle, cameraRole)
    let cameraId = camera ? camera.id : null

    return (
      <Container>
        <NotificationBanner
          visible={toggleBanner}
          bannerMessage={{
            data: stationIDs,
            message: 'docking stations detected'
          }}
          active={dockingStation}
          dockingMode={dockingModeActivated}
          onBannerButtonPress={this.callDock}
        />
        <InnerContainer
          onContextMenu={this.handleContextMenu}
          onClick={this.handleClick}
          onMouseMove={this.handleMouseMove}
        >
          <LogoContainer shadow />
          <DashboardNavbar vehicleId={vehicleId} />
          <ReverseFrame pulse={isRear} />
          <StreamContainer>
            <WebRTCStream
              // Change to camera role eventually; we're sacrificing a double
              // render until then
              cameraId={cameraId}
              vehicleId={vehicleId}
              ref={stream => {
                this.stream = stream
              }}
              addCommand={addCommand}
              onVideoStreamStart={this.resize}
              onVideoStreamLoad={this.resize}
            />
          </StreamContainer>
          <ArcControl
            manifest={manifest}
            top={videoTop}
            left={videoLeft}
            width={videoWidth}
            height={videoHeight}
            onArc={this.handleArcClick}
            ignoreModifier={EXPOSURE_MODIFIER}
          />
          <MapComponent vehicleId={vehicleId} />
          {dockingModeActivated && (
            <DockingUIContainer
              stationIDs={stationIDs}
              dockingStation={dockingStation}
              status={status}
              distanceX={distanceX}
              distanceY={distanceY}
              stopped={stopped}
              angle={angle}
              dockingFailed={dockingFailed}
              dockingSuccess={dockingSuccess}
              callDock={this.callDock}
              stopVehicle={this.stopVehicle}
            />
          )}
          <LeftControls>
            <CameraViewButton
              name="relinquish"
              handleClick={this.relinquish}
              margin
            >
              relinquish
            </CameraViewButton>
            <CameraViewButton name="stop" handleClick={this.stopVehicle} margin>
              stop
            </CameraViewButton>
            <CommandView selected={2} />
            <ExposureMessage
              mode={exposureMode}
              onToggleExposure={this.switchExposureMode}
            />
            <CommandOverrideMessage
              commandOverrideMode={isCommandOverrideMode}
              onClick={this.switchCommandOverrideMode}
            />
          </LeftControls>
          <Tint />
          <Overlay visible={helpModalIsOpen || zStageIsOpen} />
          {/*
            The exposure ring is placed directly into the CameraView instead of
            of a component of its own due to cross-cutting concerns with
            mouseMove/mouseDown events. Unfortunately, placing the exposure
            logic here makes the CameraView look more bulky than it should.
          */}
          {/* Mouseover exposure ring */}
          <ExposureRing
            active={false}
            radius={EXPOSURE_RADIUS}
            refFunc={ref => (this.mouseOverExpRing = ref)}
            onMouseDown={this.handleClickExposure}
          />
          {/* Active exposure ring */}
          <ExposureRing
            active={true}
            radius={EXPOSURE_RADIUS}
            refFunc={ref => (this.activeExpRing = ref)}
          />
          <TurnInPlaceMenu
            visible={contextMenuIsOpen}
            top={contextMenuTop}
            left={contextMenuLeft}
            handleClick={this.sendTurnInPlaceCommand}
          />
          <Help
            onClick={this.toggleHelpModal}
            onRequestClose={this.closeHelpModal}
            isOpen={helpModalIsOpen}
          />
          <ZStage
            onClick={this.toggleZStage}
            onRequestClose={this.closeZStage}
            onSendValue={this.handleSendZStage}
            isOpen={zStageIsOpen}
          />
          <WebGL
            manifest={manifest}
            displayDistortionGrid={showDistortionGrid}
            displayGroundPlane={showGroundPlane}
            displayBoundingBoxes={showBoundingBoxes}
            display3dBoundingBoxes={show3dBoundingBoxes}
            displayConvexHulls={showConvexHulls}
            boundingBoxes={boundingBoxes}
            boundingBoxes3d={boundingBoxes3d}
            convexHulls={convexHulls}
            top={videoTop}
            left={videoLeft}
            width={videoWidth}
            height={videoHeight}
          />
          <ReverseDisplay cameraRole={cameraRole} onClick={this.switchCamera} />
          {commandMessage !== null && <CommandText message={commandMessage} />}
        </InnerContainer>
      </Container>
    )
  }
}

const mapStateToProps = (state, ownProps) => {
  let vehicleId = null
  let cameraRole = null
  if (ownProps.match.params.vehicleId) {
    vehicleId = ownProps.match.params.vehicleId
  }

  if (ownProps.match.params.cameraRole) {
    cameraRole = ownProps.match.params.cameraRole
  }

  return {
    notifications: state.notifications.notifications,
    vehicleId,
    cameraRole,
    currentVehicle: vehicleDucks.currentVehicleSelector(state, { vehicleId }),
    currentCamera: vehicleDucks.currentVehicleCameraSelector(state, {
      vehicleId,
      cameraRole
    })
  }
}

const mapDispatchToProps = dispatch => {
  return bindActionCreators(
    { ...vehicleDucks, ...notificationDucks, ...commandDucks, push },
    dispatch
  )
}

export default connect(mapStateToProps, mapDispatchToProps)(CameraView)
