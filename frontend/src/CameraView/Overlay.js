import React from 'react'
import styled from 'styled-components'
import PropTypes from 'prop-types'

const Container = styled.div`
  position: absolute;
  top: 0;
  left: 0;
  width: 100%;
  height: 100%;
  z-index: 1500;
  background-color: rgba(0, 0, 0, ${props => props.alpha});
  opacity: ${props => (props.visible ? 1 : 0)};
  visibility: ${props => (props.visible ? 'visible' : 'hidden')};
  transition: 0.3s ease-in;
`

const Overlay = props => {
  return <Container alpha={props.alpha} visible={props.visible} />
}

Overlay.propTypes = {
  visible: PropTypes.bool,
  alpha: PropTypes.number
}

Overlay.defaultProps = {
  visible: false,
  alpha: 0.4
}

export default Overlay
