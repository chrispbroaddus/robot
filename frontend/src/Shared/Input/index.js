import React from 'react'
import styled from 'styled-components'
import PropTypes from 'prop-types'

const StyledInput = styled.input`
  font-family: 'Montserrat-Light', sans-serif;
  letter-spacing: 1px;
  color: #90a4ae;
  font-size: 10px;
  line-height: 13px;
  outline: 0;
  border: 0;
  border-bottom: 1px solid #cfd8dc;
  padding: 11.5px 0;
  margin: 8px 0;
  background-color: unset;

  &::placeholder {
    color: #90a4ae;
  }
`

const Input = props => {
  return (
    <StyledInput
      type="text"
      defaultValue={props.defaultValue}
      placeholder={props.placeholder}
      className={props.class}
      onClick={props.onClick}
      {...props}
    />
  )
}

Input.propTypes = {
  defaultValue: PropTypes.string,
  placeholder: PropTypes.string,
  class: PropTypes.string,
  onClick: PropTypes.func
}

Input.defaultProps = {
  defaultValue: '',
  placeholder: '',
  class: '',
  onClick: () => {}
}

export default Input
