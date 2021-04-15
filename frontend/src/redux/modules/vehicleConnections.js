// import axios from 'axios'
import { createReducer } from '../../utils'

/**
 * Actions
 */
const ADD_VEHICLE_CONNECTION = 'vehicleConnection/ADD_VEHICLE_CONNECTION'

/**
 * Reducers
 */
const reducers = {
  addVehicleConnection: (state, action) => state
}

const handlers = {
  [ADD_VEHICLE_CONNECTION]: reducers.addVehicleConnection
}

export default createReducer({}, handlers)

/**
 * Action Creators
 */
export const addVehicleConnection = (vehicleId, url, params) => ({
  type: ADD_VEHICLE_CONNECTION,
  vehicleId,
  url,
  params
})

/**
 * Side Effects
 */
export const WebSocketFactory = (
  url,
  onmessage,
  onerror,
  onstart = null,
  onend = null
) => {
  const ws = new WebSocket(url)
  ws.onmessage = onmessage
  ws.onerror = onerror
  ws.onstart = onstart
  ws.onend = onend

  return ws
}
