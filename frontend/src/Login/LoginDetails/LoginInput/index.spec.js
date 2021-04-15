import React from 'react'
import { shallow, mount } from 'enzyme'
import toJson from 'enzyme-to-json'
import LoginInput from './index'

describe('LoginInput', () => {
  let props

  beforeEach(() => {
    props = {
      value: '',
      handleChange: () => {},
      placeholder: ''
    }
  })

  it('should render self and subcomponents', () => {
    const wrapper = shallow(<LoginInput {...props} />)
    expect(toJson(wrapper)).toMatchSnapshot()
  })

  it('should render props correctly', () => {
    props.value = 'Test'
    props.handleChange = jest.fn()
    props.placeholder = 'Test placeholder'
    const wrapper = mount(<LoginInput {...props} />)
    const subject = wrapper.find('input')
    expect(subject.prop('value')).toEqual(props.value)
    expect(subject.prop('placeholder')).toEqual(props.placeholder)

    subject.simulate('change', { target: { value: 'Test description' } })
    expect(props.handleChange).toBeCalled()
  })
})
