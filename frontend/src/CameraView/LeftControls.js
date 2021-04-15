import React from 'react'
import styled from 'styled-components'
import PropTypes from 'prop-types'

const Container = styled.div`
  position: absolute;
  left: 50px;
  top: 64px;
  z-index: 800;
  & > button {
    position: relative;
    left: -8px; // This offsets the CameraViewButton's padding
  }
`

const LeftControls = props => {
  const { children } = props
  return (
    <Container>
      {children}
    </Container>
  )
}

LeftControls.propTypes = {
  children: PropTypes.node
}

LeftControls.defaultProps = {}

export default LeftControls
