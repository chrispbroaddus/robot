import React from 'react'
import { shallow } from 'enzyme'
import toJson from 'enzyme-to-json'
import { Login } from './__Container'

describe('Login', () => {
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
    const wrapper = shallow(<Login {...props} />)
    expect(toJson(wrapper)).toMatchSnapshot()
  })
})
