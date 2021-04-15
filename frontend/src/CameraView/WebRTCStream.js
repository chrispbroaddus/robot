/* global RTCPeerConnection, RTCIceCandidate, RTCSessionDescription */
import React from 'react'
import PropTypes from 'prop-types'
import styled from 'styled-components'
import WebSocketClient from '../utils/WebSocket'
import * as commandDucks from '../redux/modules/commands'
import { v4 as uuidv4 } from 'uuid'

const VideoStream = styled.video`
  width: 100%;
  height: 56.25vw; /* 100/56.25 = 1.778 */
  background: black;
  max-height: 100vh;
  max-width: 177.78vh; /* 16/9 = 1.778 */
  margin: auto;
  position: absolute;
  top: 0;
  bottom: 0; /* vertical center */
  left: 0;
  right: 0; /* horizontal center */
`

const CLIENT_ID = 'u01'
export const WEBRTC_CONFIG = {
  iceServers: [
    { urls: 'stun:stun.services.mozilla.com' },
    { urls: 'stun:stun.l.google.com:19302' }
  ]
}

/**
 * WebRTCStream sets up the WebRTC stream with the vehicle by
 */
class WebRTCStream extends React.Component {
  static propTypes = {
    cameraId: PropTypes.string,
    baseUrl: PropTypes.string,
    connectionId: PropTypes.string,
    vehicleId: PropTypes.string.isRequired,
    addCommand: PropTypes.func.isRequired,
    onVideoStreamStart: PropTypes.func.isRequired,
    onVideoStreamLoad: PropTypes.func.isRequired
  }

  static defaultProps = {
    cameraId: '',
    baseUrl: `ws${window.zippyconfig.scheme}://${window.zippyconfig
      .backend}/api/v1/ws/vehicle`,
    connectionId: uuidv4()
  }

  constructor(props) {
    super(props)

    this.ws = null
    // Ref to the <video> DOM element
    this.video = null
    // RTCPeerConnection
    this.peerconn = null
  }

  componentDidMount() {
    this.initWSConnection()
    this.props.onVideoStreamLoad()
  }

  componentWillUnmount() {
    if (this.ws) this.ws.closeSocket()
  }

  componentWillReceiveProps = nextProps => {
    // If the camera changes, request a new camera
    if (nextProps.cameraId !== this.props.cameraId) {
      this.requestCamera(nextProps.cameraId)
    }
  }

  initWSConnection() {
    const { baseUrl, vehicleId } = this.props

    const url = `${baseUrl}/${vehicleId}/user/${CLIENT_ID}/subscribe`

    try {
      this.ws = new WebSocketClient(this, url, {
        name: 'WebRTC',
        onmessage: this.handleMessage,
        onopen: this.handleWebsocketOpen
      })
    } catch (err) {
      this.ws = null
    }
  }

  /**
   * Handles a message on the websocket with the backend
   * @param  {Object} msg Message from the WebSocket connection
   */
  handleMessage = msg => {
    const signal = JSON.parse(msg.data)

    if (signal === null || signal.sdpRequest === null) return
    // The message from the WebSocket can be one of the following:
    //  - SDP request
    //  - SDP confirmation
    //  - ICE candidate
    if (signal.sdpRequest) {
      if (signal.sdpRequest.connectionID !== this.props.connectionId) {
        console.log(
          `Ignoring message for ${signal.sdpRequest.connectionID} (wanted ${this
            .props.connectionId})`
        )
        return
      }

      const offer = new RTCSessionDescription({
        type: 'offer',
        sdp: signal.sdpRequest.sdp
      })

      this.peerconn
        .setRemoteDescription(offer)
        .then(
          function() {
            // web interface always answers, never offers
            this.peerconn
              .createAnswer()
              .then(answer => {
                this.handleDescriptionCreated(
                  signal.sdpRequest.connectionID,
                  answer
                )
              })
              .catch(this.handleError)
          }.bind(this)
        )
        .catch(this.handleError)
    } else if (signal.iceCandidate) {
      if (signal.iceCandidate.connectionID !== this.props.connectionId) {
        console.log(
          `Ignoring message for ${signal.iceCandidate
            .connectionID} (wanted ${this.props.connectionId})`
        )
        return
      }

      console.log('got remote ICE candidate:', signal.iceCandidate)
      const cand = new RTCIceCandidate({
        candidate: signal.iceCandidate.candidate,
        sdpMid: signal.iceCandidate.sdp_mid,
        sdpMLineIndex: signal.iceCandidate.sdp_mline_index
      })
      this.peerconn.addIceCandidate(cand).catch(this.handleError)
    } else if (signal.sdpConfirmation) {
      if (signal.sdpConfirmation.connectionID !== this.props.connectionId) {
        console.log(
          `ignoring message for ${signal.sdpConfirmation
            .connectionID} (wanted ${this.props.connectionId})`
        )
        return
      }

      console.log('received confirmation that connection is up')
    }
  }

  handleWebsocketOpen = () => {
    const { cameraId } = this.props

    if (!cameraId) return // Don't do anything if we get a camera ID
    // kick off the handshake to establish a webrtc stream
    this.requestCamera(this.props.cameraId)
  }

  handleDescriptionCreated = (connectionID, description) => {
    this.peerconn
      .setLocalDescription(description)
      .then(
        function() {
          console.log(
            `sending SDP answer for ${connectionID}:`,
            this.peerconn.localDescription
          )

          const msg = {
            sdpRequest: {
              connectionID: connectionID,
              sdp: this.peerconn.localDescription.sdp,
              status: 0
            },
            id: uuidv4()
          }
          this.ws.send(JSON.stringify(msg))
          this.props.addCommand(
            msg.id,
            commandDucks.COMMAND_TYPE.sdpRequest,
            commandDucks.COMMAND_STATUS.sent
          )
        }.bind(this)
      )
      .catch(this.handleError)
  }

  requestCamera = cameraId => {
    // console.log(`WebRTC: Requesting cameraId ${cameraId}`)

    this.peerconn = new RTCPeerConnection(WEBRTC_CONFIG)
    this.peerconn.onicecandidate = event => {
      this.handleIceCandidate(this.props.connectionId, event)
    }
    this.peerconn.onaddstream = event => {
      this.handleRemoteStream(this.props.connectionId, event)
    }

    const msg = {
      videoRequest: {
        connectionID: this.props.connectionId,
        cameraID: cameraId
      },
      id: uuidv4()
    }

    this.ws.send(JSON.stringify(msg))
    this.props.addCommand(
      msg.id,
      commandDucks.COMMAND_TYPE.videoRequest,
      commandDucks.COMMAND_STATUS.sent
    )
  }

  /**
   * Called when an ICE Candidate is sent by the peer
   * @param  {String} connectionID Connection ID
   * @param  {Event} event         ICE event
   */
  handleIceCandidate = (connectionID, event) => {
    if (event.candidate !== null) {
      console.log(`sending ICE candidate for ${connectionID}:`, event.candidate)
      const msg = {
        iceCandidate: {
          connectionID: connectionID,
          candidate: event.candidate.candidate,
          sdpMid: event.candidate.sdpMid,
          sdpMlineIndex: event.candidate.sdpMlineIndex
        },
        id: uuidv4()
      }
      this.ws.send(JSON.stringify(msg))
      this.props.addCommand(
        msg.id,
        commandDucks.COMMAND_TYPE.iceCandidate,
        commandDucks.COMMAND_STATUS.sent
      )
    }
  }

  /**
   * Handles the event when a remote stream was sent over WebRTC. Binds the
   * stream to <video> element
   * @param  {String} connectionID Connection ID
   * @param  {Event} event         Stream event
   */
  handleRemoteStream = (connectionID, event) => {
    this.video.srcObject = event.stream

    // Let the backend know that we are connected
    const msg = {
      confirmation: {
        connectionID: connectionID,
        connected: true
      },
      id: uuidv4()
    }

    this.ws.send(JSON.stringify(msg))
    this.props.addCommand(
      msg.id,
      commandDucks.COMMAND_TYPE.confirmation,
      commandDucks.COMMAND_STATUS.sent
    )

    console.log(
      `webrtc stream for ${connectionID} now connected @ ${this.video
        .videoWidth}x${this.video.videoHeight}`
    )

    this.props.onVideoStreamStart()
  }

  /**
   * Handles errors that occur during the WebRTC negotiation process
   * @param  {Object} error Error object
   */
  handleError = error => {
    console.log(error)
  }

  render() {
    return (
      <VideoStream
        autoPlay="true"
        alt="Camera Stream"
        innerRef={video => {
          this.video = video
        }}
      />
    )
  }
}

export default WebRTCStream
