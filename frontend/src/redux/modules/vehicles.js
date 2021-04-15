import axios from 'axios'
import { createSelector } from 'reselect'
import { createReducer } from '../../utils'

/**
 * Enums
 */
export const CAMERA_ROLES = Object.freeze({
  FrontLeftStereo: Symbol.for('FrontLeftStereo'),
  FrontRightStereo: Symbol.for('FrontRightStereo'),
  RearLeftStereo: Symbol.for('RearLeftStereo'),
  RearRightStereo: Symbol.for('RearRightStereo'),
  FrontFisheye: Symbol.for('FrontFisheye'),
  RearFisheye: Symbol.for('RearFisheye'),
  LeftFisheye: Symbol.for('LeftFisheye'),
  RightFisheye: Symbol.for('RightFisheye')
})
export const VEHICLE_STATUS = Object.freeze({
  ACTIVE: 'active',
  OFFLINE: 'offline',
  INACTIVE: 'inactive',
  NETWORK_DROP: 'network_drop',
  DISABLED: 'disabled',
  LAUNCHING: 'launching' // For simulators only
})
export const VEHICLE_TYPES = Object.freeze({
  SIMULATOR: 'simulator',
  VEHICLE: 'vehicle'
})

/**
 * Actions
 */
const ADD_VEHICLE = 'vehicles/ADD_VEHICLE'
const EDIT_VEHICLE = 'vehicles/EDIT_VEHICLE'
const REMOVE_VEHICLE = 'vehicles/REMOVE_VEHICLE'
const SET_VEHICLE_STATUS = 'vehicles/SET_VEHICLE_STATUS'
const SET_VEHICLES = 'vehicles/SET_VEHICLES'
const SET_CURRENT_VEHICLE = 'vehicles/SET_CURRENT_VEHICLE'
const SET_DOCKING_DIV_LOCATION = 'vehicles/SET_DOCKING_DIV_LOCATION'

/**
 * Reducers
 */
const reducers = {
  addVehicle: (state, action) => {
    const vehicle = action.vehicle
    const newState = Object.assign({}, state)
    // This will overwrite the vehicle if it already exists
    newState.vehicles = [...newState.vehicles, vehicle]

    return newState
  },
  editVehicle: (state, action) => {
    const vehicle = Object.assign({}, action.vehicle)
    const vehicleId = action.vehicleId
    const newState = Object.assign({}, state)
    const index = newState.vehicles.findIndex(v => v.id === vehicleId)

    newState.vehicles = [
      ...newState.vehicles.slice(0, index),
      vehicle,
      ...newState.vehicles.slice(index + 1)
    ]
    return newState
  },
  removeVehicle: (state, action) => {
    const vehicleId = action.vehicleId
    const newState = Object.assign({}, state)

    const index = newState.vehicles.findIndex(v => v.id === vehicleId)
    newState.vehicles = [
      ...newState.vehicles.slice(0, index),
      ...newState.vehicles.slice(index + 1)
    ]

    return newState
  },
  setVehicleStatus: (state, action) => {
    const vehicleId = action.vehicleId
    const newState = Object.assign({}, state)
    const index = newState.vehicles.findIndex(v => v.id === vehicleId)
    const vehicle = Object.assign({}, newState.vehicles[index])
    vehicle.status = action.status

    newState.vehicles = [
      ...newState.vehicles.slice(0, index),
      vehicle,
      ...newState.vehicles.slice(index + 1)
    ]
    return newState
  },
  /**
   * Overwrites the list of vehicles with what's provided
   */
  setVehicles: (state, action) => {
    const vehicles = []
    action.vehicles.forEach(v => {
      vehicles.push(Object.assign({}, v))
    })

    return {
      ...state,
      vehicles
    }
  },
  setCurrentVehicle: (state, action) => {
    const vehicle = state.vehicles.find(v => v.id === action.vehicleId)

    return {
      ...state,
      currentVehicle: Object.assign({}, vehicle)
    }
  },
  // Send coordinates to the location of the Docking Container's top-left corner
  setDockingDivLocation: (state, action) => {
    return { ...state, dockingDivLocation: action.coordinates }
  }
}

const handlers = {
  [ADD_VEHICLE]: reducers.addVehicle,
  [EDIT_VEHICLE]: reducers.editVehicle,
  [REMOVE_VEHICLE]: reducers.removeVehicle,
  [SET_VEHICLE_STATUS]: reducers.setVehicleStatus,
  [SET_VEHICLES]: reducers.setVehicles,
  [SET_CURRENT_VEHICLE]: reducers.setCurrentVehicle,
  [SET_DOCKING_DIV_LOCATION]: reducers.setDockingDivLocation
}

export default createReducer({}, handlers)

/**
 * Action Creators
 */
export const addVehicle = vehicle => ({
  type: ADD_VEHICLE,
  vehicle
})
export const editVehicle = (vehicleId, vehicle) => ({
  type: EDIT_VEHICLE,
  vehicleId,
  vehicle
})
export const removeVehicle = vehicleId => ({
  type: REMOVE_VEHICLE,
  vehicleId
})
export const setVehicleStatus = (vehicleId, status) => ({
  type: SET_VEHICLE_STATUS,
  vehicleId,
  status
})
export const setVehicles = vehicles => {
  return {
    type: SET_VEHICLES,
    vehicles
  }
}
export const setCurrentVehicle = vehicleId => {
  return {
    type: SET_CURRENT_VEHICLE,
    vehicleId
  }
}
export const setDockingDivLocation = coordinates => {
  return {
    type: SET_DOCKING_DIV_LOCATION,
    coordinates
  }
}

/**
 * Side Effects
 */
export const fetchVehicles = () => {
  return dispatch => {
    return axios
      .get('/api/v1/vehicles', {
        withCredentials: true
      })
      .then(({ data }) => {
        dispatch(setVehicles(data.vehicles))
      })
      .catch(error => {
        return Promise.reject(`Could not fetch vehicles`)
      })
  }
}

export const fetchVehicle = vehicleId => {
  return dispatch => {
    return axios
      .get(`/api/v1/vehicle/${vehicleId}`, {
        withCredentials: true
      })
      .then(({ data }) => {
        return data.vehicle
      })
      .catch(() => {
        return Promise.reject(`Vehicle ${vehicleId} could not be found`)
      })
  }
}

/**
 * Fetches the individual vehicle's data from the API, and it will either add
 * the vehicle to the Redux store if it doesn't exist or update the record if
 * it does Redux store
 * @param  {String} vehicleId Vehicle ID
 * @return {Promise}
 */
export const fetchAndUpdateVehicle = vehicleId => {
  return (dispatch, getState) => {
    return dispatch(fetchVehicle(vehicleId))
      .then(vehicle => {
        if (getVehicleById(getState().vehicles.vehicles, vehicleId)) {
          dispatch(editVehicle(vehicle.id, vehicle))
        } else {
          dispatch(addVehicle(vehicle))
        }
      })
      .catch(() => {
        return Promise.reject(
          `Could not fetch and update vehicle ${vehicleId}.`
        )
      })
  }
}

export const relinquishVehicle = vehicleId => {
  return dispatch => {
    return axios
      .get(`/api/v1/vehicle/${vehicleId}/relinquish-control`, {
        withCredentials: true
      })
      .then(({ data }) => {
        return data
      })
      .catch(error => {
        return Promise.reject('Unable to relinquish vehicle')
      })
  }
}

export const killSimulator = vehicleId => {
  return dispatch => {
    return axios
      .put(
        `/api/v1/simulator/${vehicleId}/kill`,
        {
          name: vehicleId
        },
        {
          withCredentials: true
        }
      )
      .then(({ data }) => {
        dispatch(fetchAndUpdateVehicle(vehicleId))
      })
      .catch(error => {
        return Promise.reject(`Could not kill simulator ${vehicleId}`)
      })
  }
}

export const requestControl = vehicleId => {
  return dispatch => {
    return axios
      .get(`/api/v1/vehicle/${vehicleId}/request-control`, {
        withCredentials: true
      })
      .then(({ data }) => {
        return data
      })
      .catch(error => {
        return Promise.reject('Cannot control vehicle')
      })
  }
}

export const getLatestCameraFrame = (vehicleId, cameraId) => {
  return dispatch => {
    return axios
      .get(`/api/v1/vehicle/${vehicleId}/camera/${cameraId}?latest=true`, {
        withCredentials: true
      })
      .then(({ data }) => {
        return {
          cameraId: data.camera_id,
          timestamp: data.timestamp,
          width: data.image_width,
          height: data.image_height,
          url: data.hosted_url
        }
      })
      .catch(error => {
        if (error.response.status === 404) {
          return Promise.reject(
            `Vehicle or camera does not exist, ${vehicleId} ${cameraId}`
          )
        } else if (error.response.status === 400) {
          return Promise.reject(
            `No camera frames saved for this camera yet ${vehicleId} ${cameraId}`
          )
        }
        return Promise.reject(
          `Could not get last camera frame for ${vehicleId} ${cameraId}`
        )
      })
  }
}

export const sendExposureData = (vehicleId, x, y, r) => {
  return dispatch => {
    return axios
      .post(
        `/api/v1/vehicle/${vehicleId}/exposure`,
        {
          x,
          y,
          r
        },
        {
          withCredentials: true
        }
      )
      .then(({ data }) => {
        return data
      })
      .catch(error => {
        console.log('Error:', error)
      })
  }
}

/**
 * Returns the default "front" camera
 * @param  {Object} vehicleObject Vehicle object
 * @return {Object}               Camera object
 */
export const getFrontCamera = vehicleObject => {
  try {
    return vehicleObject.cameras.filter(
      v => v.role === Symbol.keyFor(CAMERA_ROLES.FrontFisheye)
    )[0]
  } catch (e) {
    throw new Error(
      `Could not find a camera with role "FrontFisheye" on vehicle ${vehicleObject.id}`
    )
  }
}

/**
 * Returns the default "rear" camera
 * @param  {Object} vehicleObject Vehicle object
 * @return {Object}               Camera object
 */
export const getRearCamera = vehicleObject => {
  try {
    return vehicleObject.cameras.filter(
      v => v.role === Symbol.keyFor(CAMERA_ROLES.RearFisheye)
    )[0]
  } catch (e) {
    throw new Error(
      `Could not find a camera with role "RearFisheye" on vehicle ${vehicleObject.id}`
    )
  }
}

export function getVehicleById(vehicles, vehicleId) {
  if (!Array.isArray(vehicles)) {
    throw new Error('Argument "vehicles" is not an array')
  } else if (!vehicleId || typeof vehicleId !== 'string') {
    throw new Error('Argument "vehicleId" is not valid')
  }
  return vehicles.find(v => v.id === vehicleId)
}

export const getCameraByRole = (vehicleObject, cameraRole) => {
  if (!Object.keys(CAMERA_ROLES).find(v => v === cameraRole)) {
    throw new Error('Invalid camera role')
  }

  try {
    return vehicleObject.cameras.filter(
      v => v.role === Symbol.keyFor(CAMERA_ROLES[cameraRole])
    )[0]
  } catch (e) {
    // throw new Error(
    //   `Could not find a camera with role "${cameraRole}" on vehicle`
    // )
    return null
  }
}

export const getCameraIdByRole = (vehicles, vehicleId, cameraRole) => {
  const vehicle = getVehicleById(vehicles, vehicleId)
  const camera = getCameraByRole(vehicle, cameraRole)
  return camera.id
}

/**
 * Given a camera role, determins if it's a rear camera
 * @param  {String} cameraRole Camera role
 * @return {Boolean}           True/false if it's a rear camera
 */
export const isRearCamera = cameraRole => {
  return cameraRole.toLowerCase().includes('rear'.toLowerCase())
}

/**
 * Given a vehicle and a set of cameras, find the most suitable camera to
 * "flip" to, i.e. RearFisheye -> FrontFisheye
 * @param  {Array}  cameras    Array of cameras available on the vehicle
 * @param  {String} cameraRole Camera role
 * @return {String}            Camera role
*/
export const flipCamera = (cameras, cameraRole) => {
  if (cameraRole === Symbol.keyFor(CAMERA_ROLES.FrontFisheye)) {
    return Symbol.keyFor(CAMERA_ROLES.RearFisheye)
  } else if (cameraRole === Symbol.keyFor(CAMERA_ROLES.RearFisheye)) {
    return Symbol.keyFor(CAMERA_ROLES.FrontFisheye)
  }
}

/**
 * Selectors
 */
const getVehicle = (state, props) =>
  state.vehicles.vehicles.find(v => v.id === props.vehicleId)
const getCameraForVehicleByRole = (state, props) => {
  const vehicle = state.vehicles.vehicles.find(v => v.id === props.vehicleId)
  if (!vehicle) return null
  return vehicle.cameras.find(v => v.role === props.cameraRole)
}

export const currentVehicleSelector = createSelector(
  [getVehicle],
  vehicle => vehicle
)
export const currentVehicleCameraSelector = createSelector(
  [getCameraForVehicleByRole],
  camera => camera
)
