import React from 'react'
import PropTypes from 'prop-types'
import ReactMapGL, { Marker } from 'react-map-gl'
import MapMarker from 'react-icons/lib/fa/map-marker'
import WebSocketClient from '../utils/WebSocket'
import styled from 'styled-components'

const mapWidthPx = 278
const mapHeightPx = 197

const Container = styled.div`
  position: absolute;
  right: 53px;
  z-index: 500;
`
const MapContainer = styled.div`
  box-shadow: 0 2px 5px 0 rgba(0, 0, 0, 0.5);
  transform: translateX(
    ${props => (props.visible ? '0' : `${mapWidthPx + 53}px`)}
  );
  // display: ${props => (props.visible ? 'block' : 'none')}
  transition: transform .3s ease-in;
`

const MapRestore = styled.div`
  font-family: 'Montserrat-Light';
  font-size: 10px;
  letter-spacing: 1px;
  position: absolute;
  top: -55px;
  left: 275px;
  color: white;
  padding: 6px 12px;
  border-bottom: 0;
  border-radius: 5px 5px 0 0;
  z-index: 1000;
  text-transform: uppercase;
  cursor: pointer;
  transition: 0.2s opacity, transform 0.2s;
  transform: ${props => (props.visible ? 'rotate(0deg)' : 'rotate(-90deg)')};
`

class Map extends React.Component {
  static propTypes = {
    vehicleId: PropTypes.string.isRequired
  }

  constructor(props) {
    super(props)
    this.state = {
      lat: 0,
      lon: 0,
      visible: false
    }
    this.url = `ws${window.zippyconfig.scheme}://${window.zippyconfig
      .backend}/api/v1/ws/vehicle/${this.props.vehicleId}/location/subscribe`
  }

  componentDidMount() {
    // Add an event listener for Map
    if (window) window.addEventListener('keypress', this.handleKeyDown)

    this.ws = new WebSocketClient(this, this.url, {
      name: 'Map',
      onmessage: this.handleResponse
    })
  }

  componentWillUnmount() {
    if (this.ws) this.ws.closeSocket()
  }

  handleResponse = msg => {
    const payload = JSON.parse(msg.data) // parse data and set it to payload
    this.setState({
      lat: payload.location.lat,
      lon: payload.location.lon
    })
  }

  toggleMap = evt => {
    this.setState(prevState => ({ visible: !prevState.visible }))
    if (evt) {
      evt.stopPropagation()
    }
  }

  handleKeyDown = event => {
    const key = event.key.toLowerCase()
    if (key === 'm') {
      this.toggleMap()
    }
  }

  render() {
    const { lat, lon } = this.state
    return (
      <Container>
        <MapRestore visible={!this.state.visible} onClick={this.toggleMap}>
          <MapMarker color="rgba(104, 209, 147, 1)" size={30} />
        </MapRestore>
        <MapContainer visible={this.state.visible}>
          <ReactMapGL
            mapboxApiAccessToken={process.env.REACT_APP_MAPBOX_API_KEY}
            width={mapWidthPx}
            height={mapHeightPx}
            latitude={lat}
            longitude={lon}
            zoom={16}
          >
            <Marker latitude={lat} longitude={lon}>
              <MapMarker color="rgba(88, 209, 147, 1)" size={30} />
            </Marker>
          </ReactMapGL>
        </MapContainer>
      </Container>
    )
  }
}

export default Map
