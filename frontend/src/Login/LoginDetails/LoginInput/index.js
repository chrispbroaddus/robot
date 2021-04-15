import React from 'react'
import PropTypes from 'prop-types'
import styled from 'styled-components'

const LoginField = styled.input`
  width: 210px;
  height: 30px;
  padding: 5px 10px;
  margin: 0 auto;
  margin-bottom: 10px;
  border: 1px solid #cfd8dc;
  border-radius: 5px;
  color: #90a4ae;
  font-size: 0.8rem;
  display: block;

  &::placeholder {
    color: #90a4ae;
    font-family: Montserrat-Light;
    letter-spacing: 0.5px;
  }
`

const LoginInput = props => {
  return (
    <LoginField
      name={props.name}
      type={props.type}
      value={props.value}
      onChange={props.handleChange}
      placeholder={props.placeholder}
      innerRef={props.refFunc}
    />
  )
}

LoginInput.propTypes = {
  value: PropTypes.string,
  placeholder: PropTypes.string,
  handleChange: PropTypes.func.isRequired,
  name: PropTypes.string,
  type: PropTypes.string,
  // Using the property "refFunc" because React complains when we try to use ref
  refFunc: PropTypes.func
}

LoginInput.defaultProps = {
  type: 'text'
}

export default LoginInput
