// import axios from 'axios'
import { createReducer } from '../../utils'

export const NOTIFICATION_STATUS = Object.freeze({
  SUCCESS: 'success',
  WARNING: 'warning',
  DANGER: 'danger',
  NORMAL: 'normal'
})
/**
 * Actions
 */
const ADD_NOTIFICATION = 'notifications/ADD_NOTIFICATION'
const DISMISS_NOTIFICATION = 'notifications/DISMISS_NOTIFICATION'
/**
 * Reducers
 */
const reducers = {
  addNotification: (state, action) => {
    const newState = Object.assign({}, state)
    newState.notifications = [
      ...newState.notifications,
      notificationFactory(action.message, action.status, action.timeout)
    ]
    return newState
  },
  dismissNotification: state => {
    const newState = Object.assign({}, state)
    newState.notifications = [...newState.notifications.slice(1)]
    return newState
  }
}

const handlers = {
  [ADD_NOTIFICATION]: reducers.addNotification,
  [DISMISS_NOTIFICATION]: reducers.dismissNotification
}

export default createReducer({}, handlers)

/**
 * Action Creators
 */
export const addNotification = (message, status, timeout = 3000) => ({
  type: ADD_NOTIFICATION,
  message,
  status,
  timeout
})
export const dismissNotification = () => {
  return {
    type: DISMISS_NOTIFICATION
  }
}

/**
 * Side Effects
 */
export function notificationFactory(
  message,
  status = NOTIFICATION_STATUS.normal,
  timeout = 3000
) {
  return {
    message,
    status,
    timeout
  }
}

export function notificationColor(status) {
  let bgColor
  let fgColor
  switch (status) {
    case NOTIFICATION_STATUS.SUCCESS:
      bgColor = '#2b2b2b'
      fgColor = '#50e3c2'
      break
    case NOTIFICATION_STATUS.WARNING:
      bgColor = '#2b2b2b'
      fgColor = '#f5a623'
      break
    case NOTIFICATION_STATUS.DANGER:
      bgColor = '#2b2b2b'
      fgColor = '#fe7867'
      break
    case NOTIFICATION_STATUS.NORMAL:
      bgColor = '#2b2b2b'
      fgColor = '#fff'
      break
  }

  return {
    bgColor,
    fgColor
  }
}
