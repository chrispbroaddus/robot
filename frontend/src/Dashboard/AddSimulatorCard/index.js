import React from 'react'
import PropTypes from 'prop-types'
import styled from 'styled-components'
import AddSimulatorForm from './AddSimulatorForm'

const Container = styled.div`
  &.add-simulator-card {
    height: 339px;
    width: 265px;
    border: 0.7px dashed #c5cfd4;
    border-radius: 2px;
    box-shadow: unset;
    background-color: unset;
    display: flex;
    flex-flow: column nowrap;
    cursor: pointer;
    user-select: none;
  }

  &.add-simulator-form {
    height: 248px;
    width: 353px;
    border-radius: 2px;
    background-color: #fff;
    box-shadow: -1px 2px 8px 0 rgba(155, 155, 155, 0.2);
  }
`

const AddSimulator = styled.div`
  .add-simulator {
    display: flex;
    flex-flow: column nowrap;
    flex: 1 1 auto;
  }

  & .plus,
  & .title {
    flex: 1 0 auto;
    display: flex;
    flex-flow: column nowrap;
    justify-content: center;
    text-align: center;
    color: #c8d3d6;
  }

  & .plus {
    flex: 1 1 auto;
    font-size: 9rem;
    font-weight: bold;
  }
`

class AddSimulatorCard extends React.Component {
  static propTypes = {
    title: PropTypes.string,
    src: PropTypes.string,
    class: PropTypes.string,
    status: PropTypes.string,
    id: PropTypes.string,
    cameraId: PropTypes.string,
    onComplete: PropTypes.func.isRequired
  }

  constructor(props) {
    super(props)

    this.state = {
      displayForm: false
    }
  }

  handleOnClick = () => {
    this.setState({ displayForm: true })
  }

  handleCancel = e => {
    this.setState({ displayForm: false })
  }

  render() {
    return (
      <Container
        className={`card ${this.state.displayForm
          ? 'add-simulator-form'
          : 'add-simulator-card'}`}
        onClick={this.handleOnClick}
      >
        {this.state.displayForm ? (
          <AddSimulatorForm
            handleCancel={this.handleCancel}
            onComplete={this.props.onComplete}
          />
        ) : (
          <AddSimulator>
            <div className="plus">+</div>
            <div className="title">start a simulator</div>
          </AddSimulator>
        )}
      </Container>
    )
  }
}

export default AddSimulatorCard
