import WebSocketClient from './WebSocket'

describe('WebSocketClient', () => {
  xit(
    'should successfully instantiate a websocket object with no options',
    () => {
      const url = 'ws://test.com'
      const wsc = new WebSocketClient(null, url)

      expect(wsc).toBeTruthy()
    }
  )

  it('should throw an error if the keepAliveBody is not a string', () => {
    const url = 'ws://test.com'
    const options = {
      keepAliveBody: { invalid: 1234 }
    }

    expect(() => {
      const wsc = new WebSocketClient(null, url, options) // eslint-disable-line
    }).toThrow()
  })
})
