import React from 'react'
import { shallow } from 'enzyme'
import toJson from 'enzyme-to-json'
import ZStageContainer from './__Container.js'

describe('ZStageContainer', () => {
  let props

  beforeEach(() => {
    props = {
      isOpen: true,
      onRequestClose: () => {},
      onSendValue: () => {}
    }
  })

  it('should render self and subcomponents', () => {
    const wrapper = shallow(<ZStageContainer {...props} />)
    expect(toJson(wrapper)).toMatchSnapshot()
  })
})
