import { combineReducers } from 'redux'
import { routerReducer as router } from 'react-router-redux'
import commands from './modules/commands'
import notifications from './modules/notifications'
import user from './modules/user'
import vehicles from './modules/vehicles'
import vehicleConnections from './modules/vehicleConnections'

const rootReducer = combineReducers({
  commands,
  notifications,
  user,
  vehicles,
  vehicleConnections,
  router
})

export default rootReducer
