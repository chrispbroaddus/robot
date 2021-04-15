import React from 'react'
import styled, { keyframes } from 'styled-components'
import PropTypes from 'prop-types'

const borderWidthPx = 5

const pulse = keyframes`
  0% {
    opacity: 1;
  }
  50% {
    opacity: 0.5;
  }
  100% {
    opacity: 1;
  }
`

const Container = styled.div`
  position: absolute;
  top: 0;
  left: 0;
  width: 100%;
  height: 100%;
  // Intentionally low z-index
  z-index: 100;
  box-sizing: border-box;

  ${props =>
    props.pulse &&
    `
      & {
        border: ${borderWidthPx}px solid #f5a623;
        animation: ${pulse} 2s linear infinite;
      }
    `};
`

class ReverseFrame extends React.Component {
  static propTypes = {
    pulse: PropTypes.bool
  }

  static defaultProps = {
    pulse: false
  }

  render() {
    const { pulse } = this.props

    return <Container pulse={pulse} />
  }
}

export default ReverseFrame
