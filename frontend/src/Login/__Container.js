import React from 'react'
import styled from 'styled-components'
import PropTypes from 'prop-types'
import { push } from 'react-router-redux'
import { bindActionCreators } from 'redux'
import { connect } from 'react-redux'
import LoginDetailsContainer from './LoginDetails'
import Logo from '../Shared/Logo'
import * as userDucks from '../redux/modules/user'
import ZippyIllustration from '../images/loginZippyIllustration.png'

const Container = styled.div`
  position: absolute;
  height: 100vh;
  width: 100vw;
  background-color: white;
  background-image: url('${ZippyIllustration}');
  background-size: cover;
`

const LogoContainer = styled(Logo)`
  margin-left: 43px;
  margin-top: 28px;
  display: block;
`

export const Login = props =>
  <Container>
    <LogoContainer />
    <LoginDetailsContainer {...props} />
  </Container>

Login.propTypes = {
  login: PropTypes.func.isRequired,
  push: PropTypes.func.isRequired,
  isAuthenticated: PropTypes.bool.isRequired
}

Login.defaultProps = {}

const mapStateToProps = state => {
  return {
    isAuthenticated: state.user.isAuthenticated
  }
}

const mapDispatchToProps = dispatch => {
  return bindActionCreators({ ...userDucks, push }, dispatch)
}

export default connect(mapStateToProps, mapDispatchToProps)(Login)
