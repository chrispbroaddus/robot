import React from 'react'
import { shallow, mount } from 'enzyme'
import toJson from 'enzyme-to-json'
import { Server, WebSocket } from 'mock-socket'
import MockRTC from 'rtc-mocks'
import WebRTCStream from './WebRTCStream'

describe('WebRTCStream', () => {
  const CLIENT_ID = 'u01'
  let props

  beforeEach(() => {
    props = {
      cameraId: 'testing',
      baseUrl: 'ws://localhost:8080',
      vehicleId: 'testVehicle',
      connectionId: '0987-6543-1234',
      addCommand: () => {},
      onVideoStreamStart: () => {},
      onVideoStreamLoad: () => {}
    }

    global.WebSocket = WebSocket
    global.RTCSessionDescription = jest
      .fn()
      .mockImplementation(MockRTC.RTCSessionDescription)
    global.RTCIceCandidate = jest
      .fn()
      .mockImplementation(MockRTC.RTCIceCandidate)
    global.RTCPeerConnection = jest
      .fn()
      .mockImplementation(MockRTC.RTCPeerConnection)
  })

  it('should render self and subcomponents', done => {
    const mockWS = new Server(
      `${props.baseUrl}/${props.vehicleId}/user/${CLIENT_ID}/subscribe`
    )
    const wrapper = shallow(<WebRTCStream {...props} />)
    expect(toJson(wrapper)).toMatchSnapshot()
    setTimeout(() => {
      mockWS.stop(done)
    }, 50)
  })

  it('should attempt to create a WebSocket connection to the backend', done => {
    const mockWS = new Server(
      `${props.baseUrl}/${props.vehicleId}/user/${CLIENT_ID}/subscribe`
    )
    const called = jest.fn()
    mockWS.on('connection', () => {
      called()
    })
    mockWS.on('message', () => {
      called()
    })

    mount(<WebRTCStream {...props} />)

    expect.assertions(1)
    setTimeout(() => {
      expect(called).toHaveBeenCalledTimes(2)
      mockWS.stop(done)
    }, 50)
  })

  it('should create a WebRTC PeerConnection and send a videoRequest to the server', done => {
    const messageReceivedFromServer = jest.fn()
    const mockWS = new Server(
      `${props.baseUrl}/${props.vehicleId}/user/${CLIENT_ID}/subscribe`
    )
    mockWS.on('connection', server => {
      mockWS.send(
        JSON.stringify({
          sdpRequest: {
            connectionID: '0987-6543-1234',
            sdp: {
              test: 'value'
            }
          }
        })
      )
    })
    mockWS.on('message', message => {
      const signal = JSON.parse(message)
      messageReceivedFromServer()
      expect(signal.videoRequest).toBeTruthy()
    })

    mount(<WebRTCStream {...props} />)

    expect.assertions(3)

    setTimeout(() => {
      expect(messageReceivedFromServer).toHaveBeenCalled()
      expect(global.RTCPeerConnection).toHaveBeenCalled()
      mockWS.stop(done)
    }, 50)
  })

  // Having difficulties with getting this to work because the rtc-mocks module
  // doesn't support triggering the "onicecandidate" event for the
  // RTCPeerConnection.
  xit(
    'should send an ICE candidate through the WS when the peer connection sends one to the frontend',
    done => {
      const messageReceivedFromServer = jest.fn()
      const mockWS = new Server(
        `${props.baseUrl}/${props.vehicleId}/user/${CLIENT_ID}/subscribe`
      )
      const candidate = {
        candidate: 'test',
        sdpMid: 'sdpMid',
        sdpMlineIndex: 'sdpMlineIndex'
      }
      const id = 'testIceCandidateId'

      mockWS.on('connection', server => {
        mockWS.send(
          JSON.stringify({
            sdpRequest: {
              connectionID: '0987-6543-1234',
              sdp: {
                test: 'value'
              }
            }
          })
        )
      })
      mockWS.on('message', message => {
        console.log('messsage:', message)
        const signal = JSON.parse(message)

        if (signal.videoRequest) {
          // Once we get a video request message, add an ICE candidate
          global.RTCPeerConnection.addIceCandidate(candidate, id)
        }
        if (signal.iceCandidate) {
          messageReceivedFromServer()
        }
      })

      mount(<WebRTCStream {...props} />)

      expect.assertions(2)

      setTimeout(() => {
        expect(messageReceivedFromServer).toHaveBeenCalledOnce()
        expect(global.RTCPeerConnection).toHaveBeenCalled()
        mockWS.stop()
        done()
      }, 100)
    }
  )
})
