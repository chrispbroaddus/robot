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

  &.fadeOut {
    animation-name: fadeOut;
    animation-duration: 2s;
    animation-delay: 0.2s;
    animation-fill-mode: forwards;
  }
`

class ExposureMessage extends React.Component {
  static propTypes = {
    onToggleExposure: PropTypes.func.isRequired,
    mode: PropTypes.oneOf(['auto', 'ring']).isRequired
  }

  static defaultProps = {
    isManualExposure: false
  }

  constructor(props) {
    super(props)

    this.timeout = null

    this.state = {
      fadeOut: true
    }
  }

  componentDidUpdate(prevProps) {
    if (prevProps.mode === 'ring' && this.props.mode === 'auto') {
      this.setState({ fadeOut: false })
      clearTimeout(this.timeout)
      this.timeout = setTimeout(() => {
        this.setState({ fadeOut: true })
      }, 1000)
    }
  }

  handleExposureAuto = () => {
    const { onToggleExposure } = this.props
    onToggleExposure('auto')
  }

  render() {
    const { mode } = this.props
    const { fadeOut } = this.state

    return (
      <Container>
        <StatusDisplay
          visible={mode === 'ring'}
          onClick={this.handleExposureAuto}
        >
          Exposure Ring Activated
        </StatusDisplay>
        <StatusDisplay
          className={fadeOut ? 'fadeOut' : ''}
          visible={mode === 'auto'}
          color="#ccc"
        >
          Auto Exposure
        </StatusDisplay>
      </Container>
    )
  }
}

export default ExposureMessage
