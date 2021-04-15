import React from 'react'
import { shallow } from 'enzyme'
import toJson from 'enzyme-to-json'
import Navigation from './Navigation'

describe('Navigation', () => {
  let props

  beforeEach(() => {
    props = {}
  })

  it('should render self and subcomponents', () => {
    const wrapper = shallow(<Navigation {...props} />)
    expect(toJson(wrapper)).toMatchSnapshot()
  })
})
