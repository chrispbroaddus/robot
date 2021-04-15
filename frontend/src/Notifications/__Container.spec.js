import React from 'react'
import { shallow, mount } from 'enzyme'
import toJson from 'enzyme-to-json'
import { NotificationContainer } from './__Container'
import { NOTIFICATION_STATUS } from '../redux/modules/notifications'

describe('Notifications', () => {
  let props

  beforeEach(() => {
    props = {
      visible: false,
      notifications: [],
      addNotification: jest.fn(),
      dismissNotification: jest.fn(),
      currentNotification: {
        message: '',
        timeout: 0,
        status: ''
      }
    }

    global.WebSocket = jest.fn()
  })

  it('should render self and subcomponents', () => {
    const wrapper = shallow(<NotificationContainer {...props} />)
    expect(toJson(wrapper)).toMatchSnapshot()
  })

  it('should not render when there are no notifications to display', () => {
    props.notifications = []
    props.currentNotification = null
    const wrapper = shallow(<NotificationContainer {...props} />)

    expect(toJson(wrapper)).toMatchSnapshot()
  })

  it('should render a notification if one is provided', () => {
    const notification = {
      message: 'Test message #1',
      timeout: 3000,
      status: NOTIFICATION_STATUS.SUCCESS
    }
    props.notifications = [notification]
    props.currentNotification = notification
    const wrapper = shallow(<NotificationContainer {...props} />)

    expect(toJson(wrapper)).toMatchSnapshot()
  })

  it('should dismiss a notification when clicked', () => {
    const notification = {
      message: 'Test message #1',
      timeout: 3000,
      status: NOTIFICATION_STATUS.SUCCESS
    }
    props.notifications = [notification]
    props.currentNotification = notification
    const wrapper = mount(<NotificationContainer {...props} />)

    const container = wrapper.find('div').first()
    container.simulate('click')

    expect(props.dismissNotification).toHaveBeenCalled()
    expect(toJson(wrapper)).toMatchSnapshot()
  })

  xit(
    'should dismiss a notification after the timeout has been reached',
    () => {}
  )
})
