import React from 'react'
import PropTypes from 'prop-types'
import styled, { keyframes } from 'styled-components'
import LoginInput from './LoginInput'

const Container = styled.div`
  position: relative;
  margin: auto;
  margin-top: 250px;
  text-align: center;
  padding: 10px;
  width: 80vw;
`
const fadeIn = keyframes`
  0% {
    opacity: 0;
  }
  100% {
    opacity: 1;
  }
`

const Title = styled.h1`
  margin: auto;
  margin-bottom: 20px;
  color: #f6a623;
  font-size: 48px;
`

const Subtitle = styled.h2`
  margin: auto;
  margin-bottom: 25px;
  color: #90a4ae;
  font-size: 30px;
  font-weight: 300;
`

const AuthMessage = styled.h2.attrs({
  name: 'auth-message'
})`
  margin: auto;
  margin-bottom: 25px;
  color: #fe7867;
  font-size: 16px;
  animation: 0.3s ${fadeIn};
`

const BottomText = styled.div`
  font-size: 12px;
  color: #90a4ae;
  font-weight: 300;
  letter-spacing: 0.5px;
  margin-top: 16px;
`

const LoginButton = styled.button.attrs({
  type: 'submit'
})`
  width: 114px;
  height: 38px;
  padding: 5px;
  background-color: #cfd8dc;
  border-radius: 5px;
  color: white;
  font-size: 15px;
  font-family: Montserrat-Light;
  letter-spacing: 0.5px;
  border: none;
  margin-top: 15px;
  transition: opacity 1s;

  &:hover {
    opacity: 0.5;
  }

  *:active {
    border: 1px solid #cfd8dc;
    color: #cfd8dc;
    background-color: white;
  }
`

class LoginDetails extends React.Component {
  focusField = null

  static propTypes = {
    login: PropTypes.func.isRequired,
    push: PropTypes.func.isRequired,
    isAuthenticated: PropTypes.bool.isRequired,
    location: PropTypes.shape({
      state: PropTypes.shape({
        authMessage: PropTypes.string,
        from: PropTypes.shape({
          pathname: PropTypes.string
        })
      })
    }).isRequired
  }

  constructor(props) {
    super(props)
    this.state = {
      username: '',
      password: '',
      authMessage: ''
    }
  }

  componentWillMount() {}

  componentDidMount() {
    const { location } = this.props

    setTimeout(() => {
      if (this.focusField !== null) {
        this.focusField.focus()
      }
    }, 100)

    if (location && location.state && location.state.authMessage) {
      // Read the message from the previous route and display it (if applicable)
      const { authMessage } = location.state
      if (authMessage && authMessage !== '') this.setState({ authMessage })
    }
  }

  handleSubmit = event => {
    event.preventDefault()

    if (!this.state.username || !this.state.password) return

    const defaultRoute = '/app/dashboard'
    // If a referrer was specified, navigate to that route...
    const { location } = this.props
    let route =
      location.state && location.state.from && location.state.from.pathname
        ? location.state.from.pathname
        : defaultRoute

    // if (location.state.from.pathname)
    //   console.log('referrer found', location.state.from.pathname)

    this.props
      .login(this.state.username, this.state.password)
      .then(() => {
        this.props.push(route)
      })
      .catch(message => {
        this.setState({
          authMessage: message
        })
      })
  }

  handleUsernameChange = event => {
    this.setState({ username: event.target.value })
  }

  handlePasswordChange = event => {
    this.setState({ password: event.target.value })
  }

  render() {
    const { authMessage } = this.state
    return (
      <Container>
        <Title>Welcome to Zippy Operations</Title>
        <Subtitle>please log in to get started</Subtitle>
        <form onSubmit={this.handleSubmit}>
          {authMessage &&
            <AuthMessage>
              {authMessage}
            </AuthMessage>}
          <LoginInput
            name="username"
            value={this.state.username}
            placeholder="username"
            handleChange={this.handleUsernameChange}
            refFunc={field => (this.focusField = field)}
          />
          <LoginInput
            name="password"
            type="password"
            value={this.state.password}
            placeholder="password"
            handleChange={this.handlePasswordChange}
          />
          <LoginButton>Log In</LoginButton>
          <BottomText>
            Please log in with &quot;admin&quot; as username and password
          </BottomText>
        </form>
      </Container>
    )
  }
}

export default LoginDetails
