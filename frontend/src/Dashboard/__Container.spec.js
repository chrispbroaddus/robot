import React from 'react'
import { shallow } from 'enzyme'
import toJson from 'enzyme-to-json'
import { Dashboard } from './__Container.js'

describe('Dashboard', () => {
  let props

  beforeEach(() => {
    props = {
      addVehicle: () => {},
      editVehicle: () => {},
      fetchVehicles: () => {},
      fetchAndUpdateVehicle: () => {},
      killSimulator: () => {},
      setVehicleStatus: () => {},
      getLatestCameraFrame: () => {},
      push: () => {},
      requestControl: () => {},
      addNotification: () => {},
      vehicles: []
    }
  })

  it('should render self and subcomponents', () => {
    const wrapper = shallow(<Dashboard {...props} />)
    expect(toJson(wrapper)).toMatchSnapshot()
  })
})
