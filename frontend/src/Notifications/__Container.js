import React from 'react'
import PropTypes from 'prop-types'
import styled, { keyframes } from 'styled-components'
import { bindActionCreators } from 'redux'
import { push } from 'react-router-redux'
import { connect } from 'react-redux'
import WebSocketClient from '../utils/WebSocket'
import * as notificationDucks from '../redux/modules/notifications'
import CommonShapes from '../Shared/CommonShapes'

const heightPx = 60

const AnimateIn = keyframes`
  from {
    height: 0;
    opacity: 0;
  }

  to {
    height: ${heightPx}px;
    opacity: 1;
  }
`

const AnimateOut = keyframes`
  from {
    height: ${heightPx}px;
    opacity: 1;
  }

  100% {
    height: 0;
    opacity: 0;
  }
`

const Container = styled.div`
  height: 0; // Initially hide the banner
  opacity: 0;
  background-color: ${props => (props.color ? props.color : '#2b2b2b')};
  width: 100%;
  z-index: 1;
  overflow: hidden;
  animation: 0.2s ease-in forwards
    ${props => (props.visible ? AnimateIn : AnimateOut)};
  cursor: pointer;
`

const InnerContainer = styled.div`
  padding-top: 22px;
  color: ${props => (props.color ? props.color : '#fff')};
  font-family: 'Montserrat-Regular';
  letter-spacing: 1px;
  font-size: 13px;
  text-align: center;
`

const NOTIFY_WS_URL = `ws${window.zippyconfig.scheme}://${window.zippyconfig
  .backend}/api/v1/ws/operator/notify`

export class NotificationContainer extends React.Component {
  static propTypes = {
    visible: PropTypes.bool,
    notifications: PropTypes.arrayOf(PropTypes.object).isRequired,
    addNotification: PropTypes.func.isRequired,
    dismissNotification: PropTypes.func.isRequired,
    currentNotification: CommonShapes.Notifications
  }

  static defaultProps = {
    visible: false
  }

  constructor(props) {
    super(props)
    this.timeout = null
    this.notificationWS = null
  }

  componentDidMount() {
    this.notificationWS = new WebSocketClient(this, NOTIFY_WS_URL, {
      name: 'Notifications',
      onmessage: this.handleWSMessage
    })
  }

  componentWillUpdate(nextProps) {
    const { dismissNotification, currentNotification } = nextProps
    if (currentNotification !== null) {
      if (currentNotification.timeout > 0) {
        if (this.timeout) clearTimeout(this.timeout)
        this.timeout = setTimeout(
          dismissNotification,
          currentNotification.timeout
        )
      }
    }
  }

  componentWillUnmount() {
    if (this.notificationWS) this.notificationWS.closeSocket()
  }

  /**
   * Handles the messages coming from the notification websocket
   * @param  {Object} message Message from the ws
   */
  handleWSMessage = msg => {
    const { addNotification } = this.props
    const message = JSON.parse(msg.data)
    // Expecting the message to take the shape:
    //   message: {
    //     type: String,
    //     message: String,
    //     payload: String,
    //   }

    if (message.type === 'vehicle_disconnected') {
      // Display a message indefinitely when disconnected
      addNotification(
        message.message,
        notificationDucks.NOTIFICATION_STATUS.DANGER,
        0
      )
    }
  }

  handleClick = () => {
    if (this.timeout) clearTimeout(this.timeout)
    this.props.dismissNotification()
  }

  render() {
    const { visible, currentNotification } = this.props
    let bgColor = null,
      fgColor = null
    if (currentNotification) {
      const { bgColor: bg, fgColor: fg } = notificationDucks.notificationColor(
        currentNotification.status
      )
      bgColor = bg
      fgColor = fg
    }

    return (
      <Container
        visible={visible}
        onClick={this.handleClick}
        color={bgColor}
        title="Click to dismiss notification"
      >
        <InnerContainer color={fgColor}>
          {currentNotification && currentNotification.message}
        </InnerContainer>
      </Container>
    )
  }
}

const mapStateToProps = state => {
  const notifications = state.notifications.notifications
  let currentNotification
  if (notifications.length > 0) {
    currentNotification = notifications[0]
  } else {
    currentNotification = null
  }
  return {
    notifications: notifications,
    currentNotification,
    visible: notifications.length > 0
  }
}

const mapDispatchToProps = dispatch => {
  return bindActionCreators({ ...notificationDucks, push }, dispatch)
}

export default connect(mapStateToProps, mapDispatchToProps)(
  NotificationContainer
)
