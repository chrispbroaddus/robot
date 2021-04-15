import React from 'react'
import PropTypes from 'prop-types'
import styled, { keyframes } from 'styled-components'

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
const Button = styled.button`
  color: white;
  border: 0;
  outline: 0;
  height: 34px;
  width: 124px;
  border-radius: 5px;
  background: none;
  font-family: 'Montserrat-Light';
  letter-spacing: 1px;
  font-size: 10px;
  text-transform: uppercase;
  display: block;
  margin-top: 7px;
  text-align: left;
  cursor: pointer;
  padding: 3px 8px;

  &:active {
    opacity: 0.5;
  }

  ${props =>
    props.margin &&
    `
      margin-top: 7px;
      position: relative;
    `};
  ${props =>
    props.statePersist === true &&
    `
      background-color: #f5a623;
      border-color: #f5a623;
      animation: ${pulse} 2s linear infinite;
    `};
`

const CameraViewButton = props => {
  return (
    <Button
      name={props.name}
      onClick={props.handleClick}
      margin={props.margin}
      statePersist={props.statePersist}
    >
      {props.children}
    </Button>
  )
}

CameraViewButton.propTypes = {
  name: PropTypes.string,
  children: PropTypes.node,
  handleClick: PropTypes.func.isRequired,
  margin: PropTypes.bool,
  statePersist: PropTypes.bool
}

CameraViewButton.defaultProps = {
  name: '',
  margin: false,
  children: null,
  statePersist: false
}

export default CameraViewButton
