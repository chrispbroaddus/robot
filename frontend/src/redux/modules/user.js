import axios from 'axios'
import { createReducer } from '../../utils'

/**
 * Actions
 */
const AUTH_REQUEST = 'user/AUTH_REQUEST'
const AUTH_SUCCESS = 'user/AUTH_SUCCESS'
const AUTH_FAIL = 'user/AUTH_FAIL'
const AUTH_TOKEN_EXISTS = 'user/AUTH_TOKEN_EXISTS'

/**
 * Reducers
 */
const reducers = {
  authRequest: state => ({
    ...state,
    authenticating: true,
    isAuthenticated: false,
    username: null
  }),
  authSuccess: (state, action) => ({
    ...state,
    authenticating: false,
    isAuthenticated: true,
    username: action.username
  }),
  authFail: (state, action) => ({
    ...state,
    authenticating: false,
    isAuthenticated: false,
    username: null,
    message: action.message
  }),
  authTokenExists: (state, action) => ({
    ...state,
    authenticating: false,
    isAuthenticated: true,
    username: null
  })
}

const handlers = {
  [AUTH_REQUEST]: reducers.authRequest,
  [AUTH_SUCCESS]: reducers.authSuccess,
  [AUTH_FAIL]: reducers.authFail,
  [AUTH_TOKEN_EXISTS]: reducers.authTokenExists
}

export default createReducer({}, handlers)

/**
 * Action Creators
 */
export const authRequest = () => ({
  type: AUTH_REQUEST
})
export const authSuccess = user => {
  return {
    type: AUTH_SUCCESS,
    user
  }
}
export const authFail = message => {
  return {
    type: AUTH_FAIL,
    message
  }
}
export const authTokenExists = () => {
  return {
    type: AUTH_TOKEN_EXISTS
  }
}

/**
 * Side Effects
 */
export const login = (username, password) => {
  return dispatch => {
    dispatch(authRequest())

    return axios
      .post(
        '/api/v1/login',
        {
          userName: username,
          password
        },
        {
          headers: { 'Content-Type': 'application/json' }
        }
      )
      .then(
        data => {
          dispatch(authSuccess({ username }))
        },
        error => {
          if (error.response) {
            if (error.response.status === 404) {
              return Promise.reject(
                'Sorry, the username or password is invalid.'
              )
            } else {
              return Promise.reject('Could not authenticate.')
            }
          } else if (error.request) {
            return Promise.reject('Could not contact server.')
          }
          dispatch(authFail(error))
        }
      )
  }
}
