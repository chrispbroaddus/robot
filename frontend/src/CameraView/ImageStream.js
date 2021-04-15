// TODO: Cleanup - Component is now unused
import React from 'react'
import PropTypes from 'prop-types'
import WebSocketClient from '../utils/WebSocket'
import DefaultImage from '../images/default.png'

class ImageStream extends React.Component {
  ws
  img

  static propTypes = {
    websocketURL: PropTypes.string.isRequired
  }

  constructor(props) {
    super(props)
    this.state = {
      imgSrc: ''
    }
  }

  componentDidMount() {
    this.ws = new WebSocketClient(this, this.props.websocketURL, {
      onmessage: this.handleMessage
    })
    // this.ws = new WebSocket(this.props.websocketURL)
    // this.ws.onmessage = this.handleMessage
    // this.ws.onerror = err =>
    //   console.log('WebSocket error on the ImageStream:', err)
  }

  componentWillUnmount() {
    if (this.ws) this.ws.close()
  }

  handleMessage = msg => {
    const payload = JSON.parse(msg.data)
    if (payload.frame) {
      const { content } = payload.frame
      this.img.src = content
        ? `data:image/jpeg;base64,${content}`
        : DefaultImage
    }
  }

  onError = () => {
    this.img.src = DefaultImage
  }

  render() {
    return (
      <img
        src={DefaultImage}
        onError={this.onError}
        className="image-video"
        alt="Camera Stream"
        ref={img => {
          this.img = img
        }}
      />
    )
  }
}

export default ImageStream
