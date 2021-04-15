import { createReducer } from '../../utils'

/**
 * Enums
 */
// If you're adding a command type here, remember to add a human-readable entry
// to commandTypeDecoder also!
export const COMMAND_TYPE = Object.freeze({
  pointAndGo: Symbol.for('COMMAND_TYPE.POINTANDGO'),
  videoRequest: Symbol.for('COMMAND_TYPE.VIDEOREQUEST'),
  iceCandidate: Symbol.for('COMMAND_TYPE.ICECANDIDATE'),
  sdpRequest: Symbol.for('COMMAND_TYPE.SDPREQUEST'),
  confirmation: Symbol.for('COMMAND_TYPE.CONFIRMATION'),
  dockCommand: Symbol.for('COMMAND_TYPE.DOCKCOMMAND'),
  exposure: Symbol.for('COMMAND_TYPE.EXPOSURE'),
  resetExposure: Symbol.for('COMMAND_TYPE.RESETEXPOSURE'),
  stopVehicle: Symbol.for('COMMAND_TYPE.STOPVEHICLE'),
  zStage: Symbol.for('COMMAND_TYPE.ZSTAGE'),
  turnInPlace: Symbol.for('COMMAND_TYPE.TURNINPLACE'),
  pointAndGoAndTurnInPlace: Symbol.for('COMMAND_TYPE.POINTANDGOANDTURNINPLACE')
})
export const COMMAND_STATUS = Object.freeze({
  sent: Symbol.for('COMMAND_STATUS.SENT'),
  executed: Symbol.for('COMMAND_STATUS.EXECUTED'),
  confirmed: Symbol.for('COMMAND_STATUS.CONFIRMED')
})

/**
 * Actions
 */
const ADD_COMMAND = 'commands/ADD_COMMAND'
const REMOVE_COMMAND = 'commands/REMOVE_COMMAND'
const SET_COMMAND_STATUS = 'commands/SET_COMMAND_STATUS'

/**
 * Reducers
 */
const reducers = {
  addCommand: (state, action) => {
    const command = commandFactory(
      action.commandId,
      action.commandType,
      action.status
    )
    return [...state, command]
  },
  removeCommand: (state, action) => {
    const index = state.findIndex(v => v.id === action.commandId)
    return [...state.slice(index), ...state.slice(index + 1)]
  },
  setCommandStatus: (state, action) => {
    const index = state.findIndex(v => v.id === action.commandId)
    const command = Object.assign({}, state[index])
    command.status = Symbol.keyFor(action.status)

    return [...state.slice(0, index), command, ...state.slice(index + 1)]
  }
}

const handlers = {
  [ADD_COMMAND]: reducers.addCommand,
  [REMOVE_COMMAND]: reducers.removeCommand,
  [SET_COMMAND_STATUS]: reducers.setCommandStatus
}

export default createReducer({}, handlers)

/**
 * Action Creators
 */
export const addCommand = (commandId, commandType, status) => ({
  type: ADD_COMMAND,
  commandId,
  commandType,
  status
})
export const removeCommand = commandId => {
  return {
    type: REMOVE_COMMAND,
    commandId
  }
}
export const setCommandStatus = (commandId, status) => {
  return {
    type: SET_COMMAND_STATUS,
    commandId,
    status
  }
}

/**
 * Side Effects
 */
export function commandFactory(
  commandId,
  commandType = null,
  commandStatus = COMMAND_STATUS.sent
) {
  if (!Object.values(COMMAND_TYPE).includes(commandType)) {
    throw new Error('Command type is invalid.')
  }
  // Check that the command status value is one of the enum values
  if (!Object.values(COMMAND_STATUS).includes(commandStatus)) {
    throw new Error('Command status is invalid.')
  }

  return {
    id: commandId,
    type: Symbol.keyFor(commandType),
    status: Symbol.keyFor(commandStatus)
  }
}

/**
 * Human-readable decodings of command types
 * @param  {String} commandType Command type string
 * @return {String}             Human-readable command type string
 */
export function commandTypeDecoder(commandType) {
  switch (Symbol.for(commandType)) {
    case Symbol.for('COMMAND_TYPE.POINTANDGO'):
      return 'Point and Go'
    case Symbol.for('COMMAND_TYPE.VIDEOREQUEST'):
      return 'Video Feed Request'
    case Symbol.for('COMMAND_TYPE.ICECANDIDATE'):
    case Symbol.for('COMMAND_TYPE.SDPREQUEST'):
    case Symbol.for('COMMAND_TYPE.CONFIRMATION'):
      return 'Connection Request'
    case Symbol.for('COMMAND_TYPE.DOCKCOMMAND'):
      return 'Dock Command'
    case Symbol.for('COMMAND_TYPE.EXPOSURE'):
      return 'Exposure'
    case Symbol.for('COMMAND_TYPE.RESETEXPOSURE'):
      return 'Reset Exposure'
    case Symbol.for('COMMAND_TYPE.STOPVEHICLE'):
      return 'Stop Vehicle'
    case Symbol.for('COMMAND_TYPE.ZSTAGE'):
      return 'Z-Stage'
    case Symbol.for('COMMAND_TYPE.TURNINPLACE'):
      return 'Turn In Place'
    case Symbol.for('COMMAND_TYPE.POINTANDGOANDTURNINPLACE'):
      return 'Point and Go and Turn In Place'
    default:
      return null
  }
}
