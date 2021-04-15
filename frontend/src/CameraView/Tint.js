import React from 'react'
import styled from 'styled-components'
import PropTypes from 'prop-types'

const Container = styled.div`
  position: absolute;
  top: 0;
  left: 0;
  width: 100%;
  height: 100%;
  // Intentionally low z-index
  z-index: 90;
  background: rgba(0, 0, 0, ${props => props.alpha});
`

const Tint = props => {
  return <Container alpha={props.alpha} />
}

Tint.propTypes = {
  alpha: PropTypes.number
}

Tint.defaultProps = {
  alpha: 0.27
}

export default Tint
