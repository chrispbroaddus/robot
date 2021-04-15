import React from 'react'
import PropTypes from 'prop-types'
import styled from 'styled-components'
import axios from 'axios'
import Input from '../../../Shared/Input'
import { Dropdown, DropdownItem } from '../../../Shared/Dropdown'
import DashboardButton from '../../DashboardButton'

const Container = styled.div`
  margin: 0 43px;

  & input {
    width: 100%;
  }

  & h4 {
    font-family: 'Montserrat-Light';
    font-size: 14px;
    line-height: 18px;
    text-align: center;
    color: #92a4ae;
    margin: 23px 0;
  }

  & select {
    border: 1px solid #92a4ae;
    background-color: unset;
    margin: 0 10px;
    padding: 10px;
  }
`

class AddSimulatorForm extends React.Component {
  static propTypes = {
    title: PropTypes.string,
    src: PropTypes.string,
    class: PropTypes.string,
    status: PropTypes.string,
    id: PropTypes.string,
    cameraId: PropTypes.string,
    handleCancel: PropTypes.func.isRequired,
    onComplete: PropTypes.func.isRequired
  }

  constructor(props) {
    super(props)

    this.state = {
      simulators: []
    }
  }

  componentDidMount() {
    axios
      .get('/api/v1/simulators', {
        withCredentials: true
      })
      .then(({ data }) => {
        this.setState({
          simulators: data
        })
        // console.log('simulators returned', data)
      })
      .catch(() => {
        console.log('Could not reach API. Using default simulators.')
        this.setState({
          simulators: [
            'Docking Simulator',
            'Single Driving',
            'Delivery',
            'Pick Up'
          ]
        })
      })
  }

  handleClick = e => {
    if (
      this.state.name === '' ||
      !this.state.type ||
      this.state.type.title === ''
    ) {
      return
    }
    this.setState({ disabled: true })
    axios
      .post(
        '/api/v1/simulator/start',
        {
          name: this.state.name,
          type: this.state.type.title
        },
        {
          withCredentials: true
        }
      )
      // // Uncomment the two lines below for testing without actually spinning up a simulator
      // const p = new Promise(resolve => resolve({ data: 'Test data' }))
      // p
      .then(({ data }) => {
        this.setState({ disabled: false }, () => {
          this.props.handleCancel()
          this.props.onComplete(this.state.name)
        })
      })
      .catch(err => {
        this.setState({ disabled: false })
      })
  }

  handleChange = evt => {
    this.setState({
      name: evt.target.value
    })
  }

  handleSeletionChange = selection => {
    this.setState({
      type: selection
    })
  }

  render() {
    return (
      <Container>
        <h4>Add a Simulator</h4>
        <div>
          <Input placeholder="Name of simulator" onChange={this.handleChange} />
        </div>
        <Dropdown
          placeholder="Type of simulator"
          onSelect={this.handleSeletionChange}
        >
          {this.state.simulators.map((item, idx) => (
            <DropdownItem key={idx} title={item} value={idx}>
              {item}
            </DropdownItem>
          ))}
          {/*
          <DropdownItem title="Docking Simulator" value="1">
            Docking Simulator
          </DropdownItem>
          <DropdownItem title="Single Driving" value="2">
            Single Driving
          </DropdownItem>
          <DropdownItem title="Delivery" value="3">
            Delivery
          </DropdownItem>
          <DropdownItem title="Pick Up" value="4">
            Pick Up
          </DropdownItem>
          */}
        </Dropdown>
        <div className="center">
          <DashboardButton
            disabled={this.state.disabled}
            onClick={this.handleClick}
          >
            start
          </DashboardButton>
        </div>
      </Container>
    )
  }
}

export default AddSimulatorForm
