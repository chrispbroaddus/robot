import React from 'react'
import styled from 'styled-components'
import PropTypes from 'prop-types'

const StyledDropdownContainer = styled.div`
  position: absolute;
  width: 100%;
  border-radius: 2px;
  background-color: #ffffff;
  box-shadow: -1px 2px 8px 0 rgba(155, 155, 155, 0.2);
  z-index: 1000;
`

const DropdownContainer = props => {
  return (
    <StyledDropdownContainer className={props.className} {...props}>
      {props.children}
    </StyledDropdownContainer>
  )
}

DropdownContainer.propTypes = {
  children: PropTypes.node.isRequired,
  className: PropTypes.string
}

DropdownContainer.defaultProps = {
  className: ''
}

export default DropdownContainer
