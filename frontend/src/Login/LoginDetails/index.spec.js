import React from 'react'
import { mount, shallow } from 'enzyme'
import toJson from 'enzyme-to-json'
import LoginDetails from './index'

describe('LoginDetails', () => {
  let props

  beforeEach(() => {
    props = {
      login: () => {},
      push: () => {},
      isAuthenticated: false,
      location: {
        state: {}
      }
    }
  })

  it('should render self and subcomponents', () => {
    const wrapper = shallow(<LoginDetails {...props} />)
    expect(toJson(wrapper)).toMatchSnapshot()
  })

  it('should set the authMessage state if a message was provided by a previous route', () => {
    const message = 'Test message'
    props.location = {
      state: {
        authMessage: message
      }
    }

    const wrapper = mount(<LoginDetails {...props} />)
    expect(wrapper.state('authMessage')).toBeTruthy()
  })

  it('should display the message from the previous route', () => {
    const message = 'Test message'
    props.location = {
      state: {
        authMessage: message
      }
    }

    const wrapper = mount(<LoginDetails {...props} />)
    const target = wrapper.find('h2[name="auth-message"]')
    expect(target.exists()).toBe(true)
    expect(target.text()).toEqual(message)
  })

  it('should display the login form', () => {
    const wrapper = mount(<LoginDetails {...props} />)
    const usernameField = wrapper.find('input[name="username"]')
    const passwordField = wrapper.find('input[name="password"]')
    const submit = wrapper.find('button[type="submit"]')
    const form = wrapper.find('form')
    expect(usernameField.exists()).toBe(true)
    expect(passwordField.exists()).toBe(true)
    expect(submit.exists()).toBe(true)
    expect(form.exists()).toBe(true)
  })

  it('should change the state when fields are changed', () => {
    const username = 'testUser'
    const password = 'myTestPassw0rd'

    const wrapper = mount(<LoginDetails {...props} />)
    const usernameField = wrapper.find('input[name="username"]')
    const passwordField = wrapper.find('input[name="password"]')
    usernameField.simulate('change', { target: { value: username } })
    passwordField.simulate('change', { target: { value: password } })

    expect(wrapper.state('username')).toEqual(username)
    expect(wrapper.state('password')).toEqual(password)
  })

  it('should call the login() prop when a username and password is provided', () => {
    props.login = jest.fn().mockReturnValue(
      new Promise(
        (resolve, reject) => {
          resolve()
        },
        (resolve, reject) => {
          reject()
        }
      )
    )
    props.push = jest.fn()
    const username = 'testUser'
    const password = 'myTestPassw0rd'

    const wrapper = mount(<LoginDetails {...props} />)
    const form = wrapper.find('form')
    wrapper.setState({
      username,
      password
    })
    form.simulate('submit')

    expect(props.login).toHaveBeenCalledWith(username, password)
  })

  it('should not call the login() prop when a username or password is missing', () => {
    props.login = jest.fn().mockReturnValue(
      new Promise(
        (resolve, reject) => {
          resolve()
        },
        (resolve, reject) => {
          reject()
        }
      )
    )
    props.push = jest.fn()
    const username = 'testUser'
    const password = 'myTestPassw0rd'

    const wrapper = mount(<LoginDetails {...props} />)
    const form = wrapper.find('form')

    // Password missing
    wrapper.setState({
      username,
      password: ''
    })
    form.simulate('submit')

    expect(props.login).not.toHaveBeenCalled()

    // Username missing
    wrapper.setState({
      username: '',
      password
    })
    form.simulate('submit')

    expect(props.login).not.toHaveBeenCalled()

    // Both missing
    wrapper.setState({
      username: '',
      password: ''
    })
    form.simulate('submit')

    expect(props.login).not.toHaveBeenCalled()
  })

  it('should not navigate when login fails', () => {
    const authMessage = 'failed login'
    props.push = jest.fn()
    props.login = jest.fn().mockReturnValue(
      new Promise((resolve, reject) => {
        expect(props.push).not.toHaveBeenCalled()
        reject(authMessage)
      })
    )

    const wrapper = mount(<LoginDetails {...props} />)
    const form = wrapper.find('form')
    wrapper.setState({
      username: 'testUser',
      password: 'myTestPassw0rd'
    })
    form.simulate('submit')

    expect.assertions(1)
  })

  it('should display the authMessage on fail', done => {
    const authMessage = 'failed login'
    props.push = jest.fn()
    props.login = jest.fn().mockReturnValue(
      new Promise((resolve, reject) => {
        expect(props.push).not.toHaveBeenCalled()
        reject(authMessage)
      })
    )

    const wrapper = mount(<LoginDetails {...props} />)
    const form = wrapper.find('form')
    wrapper.setState({
      username: 'testUser',
      password: 'myTestPassw0rd'
    })
    form.simulate('submit')

    expect.assertions(2)
    setTimeout(() => {
      expect(wrapper.state('authMessage')).toBe(authMessage)
      done()
    })
  })

  it('should navigate to the dashboard after a successful login', done => {
    props.login = jest.fn().mockReturnValue(
      new Promise((resolve, reject) => {
        resolve()
      })
    )
    props.push = jest.fn()

    const wrapper = mount(<LoginDetails {...props} />)
    const form = wrapper.find('form')
    wrapper.setState({
      username: 'testUser',
      password: 'myTestPassw0rd'
    })
    form.simulate('submit')

    expect.assertions(1)
    setTimeout(() => {
      expect(props.push).toHaveBeenCalledWith('/app/dashboard')
      done()
    })
  })

  it('should navigate to a custom route after a successful login', done => {
    const pathname = 'test/path/name'
    props.location = {
      state: {
        from: {
          pathname
        }
      }
    }
    props.login = jest.fn().mockReturnValue(
      new Promise((resolve, reject) => {
        resolve()
      })
    )
    props.push = jest.fn()

    const wrapper = mount(<LoginDetails {...props} />)
    const form = wrapper.find('form')
    wrapper.setState({
      username: 'testUser',
      password: 'myTestPassw0rd'
    })
    form.simulate('submit')

    expect.assertions(2)
    setTimeout(() => {
      expect(props.push).not.toHaveBeenCalledWith('/app/dashboard')
      expect(props.push).toHaveBeenCalledWith(pathname)
      done()
    })
  })
})
