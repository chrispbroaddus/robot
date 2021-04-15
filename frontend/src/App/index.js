import React from 'react'
import PropTypes from 'prop-types'
import { connect } from 'react-redux'
import { bindActionCreators } from 'redux'
import { push } from 'react-router-redux'
import { Switch, Route } from 'react-router-dom'
import axios from 'axios'
import { Cookies } from 'react-cookie'
import * as userDucks from '../redux/modules/user'
import ProtectedRoute from './ProtectedRoute'
import NotificationsContainer from '../Notifications/__Container'
import CameraViewContainer from '../CameraView/__Container'
import DashboardContainer from '../Dashboard/__Container'
import LoginContainer from '../Login/__Container'

/**
 * Setup application-wide defaults for Axios
 */
axios.defaults.baseURL = `http${window.zippyconfig.scheme}://${window
  .zippyconfig.backend}`
// Allow for sharing session cookies cross-domain
axios.defaults.withCredentials = true
// Expect all POST requests to be in JSON unless specified
axios.defaults.headers.post['Content-Type'] = 'application/json'

class App extends React.Component {
  static propTypes = {
    push: PropTypes.func.isRequired,
    authTokenExists: PropTypes.func.isRequired,
    cookies: PropTypes.instanceOf(Cookies).isRequired
  }

  componentWillMount() {
    const { cookies, authTokenExists } = this.props
    // If an auth cookie exists, safely assume that the user has been
    // authenticated before. This is OK because any request made with an invalid
    // cookie will result in a redirect to the login page
    if (cookies.get('auth') !== null) authTokenExists()

    // Add a response interceptor
    axios.interceptors.response.use(
      response => {
        return response
      },
      error => {
        if (error.response && error.response.status === 401) {
          // Unauthorized, redirect to the login page
          this.props.push('/login')
        }
        return Promise.reject(error)
      }
    )
  }

  render() {
    return (
      <main>
        <NotificationsContainer />
        <Switch>
          <Route exact path="/" component={DashboardContainer} />
          <Route path="/login" component={LoginContainer} />
          <ProtectedRoute
            path="/app/dashboard"
            component={DashboardContainer}
          />
          <ProtectedRoute
            path="/app/vehicle/:vehicleId/:cameraRole"
            component={CameraViewContainer}
          />
        </Switch>
      </main>
    )
  }
}

const mapStateToProps = state => {
  return {}
}

const mapDispatchToProps = dispatch => {
  return bindActionCreators({ ...userDucks, push }, dispatch)
}

export default connect(mapStateToProps, mapDispatchToProps)(App)
