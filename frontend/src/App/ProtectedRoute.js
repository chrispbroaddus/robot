import React from 'react'
import PropTypes from 'prop-types'
import { bindActionCreators } from 'redux'
import { connect } from 'react-redux'
import { Route, Redirect } from 'react-router-dom'
import * as userDucks from '../redux/modules/user'

const redirectRoute = '/login'

/**
 * If a user lands on a page that does not make an authenticated request to
 * the server (and therefore validating their cookie/authentication status),
 * rely on the Redux state's isAuthenticated flag to decide whether or not the
 * route should be displayed
 */
const ProtectedRoute = ({ component: Component, ...rest }) => {
  return (
    <Route
      {...rest}
      render={props =>
        rest.isAuthenticated === true
          ? <Component {...props} />
          : <Redirect
              to={{
                pathname: redirectRoute,
                state: {
                  from: props.location,
                  authMessage: 'Please login again.'
                }
              }}
            />}
    />
  )
}

ProtectedRoute.propTypes = {
  component: PropTypes.func.isRequired,
  location: PropTypes.object.isRequired,
  isAuthenticated: PropTypes.bool
}

ProtectedRoute.defaultProps = {
  isAuthenticated: false
}

const mapStateToProps = (state, ownProps) => {
  return {
    isAuthenticated: state.user.isAuthenticated
  }
}

const mapDispatchToProps = dispatch => {
  return bindActionCreators({ ...userDucks }, dispatch)
}

export default connect(mapStateToProps, mapDispatchToProps)(ProtectedRoute)
