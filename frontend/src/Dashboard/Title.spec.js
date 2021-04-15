import React from 'react'
import { shallow } from 'enzyme'
import toJson from 'enzyme-to-json'
import Title from './Title'

describe('Title', () => {
  let props

  beforeEach(() => {
    props = {}
  })

  it('should render self and subcomponents', () => {
    const wrapper = shallow(<Title {...props} />)
    expect(toJson(wrapper)).toMatchSnapshot()
  })
})
