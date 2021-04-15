/**
 * Use this as a template for adding new components to the CameraView
 */
import React from 'react'
import PropTypes from 'prop-types'
import styled from 'styled-components'

const Container = styled.div`
  z-index: 900;
  position: absolute;
  left: 0;
  bottom: 0;
`

const CommandView = props => {
  const { handleClick } = props
  return <Container onClick={handleClick}>Stub</Container>
}

CommandView.propTypes = {
  handleClick: PropTypes.func
}

CommandView.defaultProps = {
  handleClick: () => {}
}

export default CommandView
