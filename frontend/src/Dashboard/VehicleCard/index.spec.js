import React from 'react'
import { shallow } from 'enzyme'
import toJson from 'enzyme-to-json'
import VehicleCard from './index'

describe('VehicleCard', () => {
  let props

  beforeEach(() => {
    props = {
      cameraRole: '',
      vehicleId: '',
      realVehicleId: '',
      status: '',
      type: '',
      handleClickOperate: () => {},
      handleClickShutdown: () => {},
      getLatestCameraFrame: () => {
        return new Promise(() => {})
      }
    }
  })

  it('should render self and subcomponents', () => {
    const wrapper = shallow(<VehicleCard {...props} />)
    expect(toJson(wrapper)).toMatchSnapshot()
  })
})
