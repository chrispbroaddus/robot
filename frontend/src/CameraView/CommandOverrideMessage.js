import React from 'react'
import styled from 'styled-components'
import PropTypes from 'prop-types'

const Container = styled.div`
  position: relative;
  width: 300px;
`

const StatusDisplay = styled.div`
  position: absolute;
  left: 0;
  top: 0;
  font-size: 8px;
  line-height: 10px;
  color: ${props => (props.color ? props.color : '#f5a623')};
  margin-top: 12px;
  letter-spacing: 1px;
  text-transform: uppercase;
  opacity: ${props => (props.visible ? 1 : 0)};
  z-index: ${props => (props.visible ? 100 : -100)};
  transition: 0.3s;
  cursor: pointer;
  user-select: none;
`

class CommandOverrideMessage extends React.PureComponent {
  static propTypes = {
    onClick: PropTypes.func.isRequired,
    commandOverrideMode: PropTypes.bool.isRequired
  }

  static defaultProps = {}

  constructor(props) {
    super(props)
  }

  render() {
    const { commandOverrideMode } = this.props

    return (
      <Container>
        <StatusDisplay
          visible={commandOverrideMode === true}
          onClick={this.onClick}
        >
          Command Override Mode
        </StatusDisplay>
      </Container>
    )
  }
}

export default CommandOverrideMessage
