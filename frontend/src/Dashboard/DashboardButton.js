import React from 'react'
import styled from 'styled-components'
import PropTypes from 'prop-types'

const StyledButton = styled.button`
  &,
  &.inactive {
    display: block;
    padding: 10px 15px;
    margin: 5px auto;
    margin-bottom: 6px;
    min-width: 118px;
    border: 1px solid #50e3c2;
    border-radius: 8px;
    color: #50e3c2;
    text-align: center;
    text-decoration: none;
    background-color: unset;
    cursor: pointer;
    font-family: Montserrat-Light;
    font-size: 12px;
    letter-spacing: 0.5px;
  }

  &.offline {
    border: 1px solid #f65045;
    color: #f65045;
  }

  &.active {
    border: 1px solid #50e3c2;
    color: #50e3c2;
  }

  &:hover {
    opacity: 0.5;
  }

  &.disabled,
  &:disabled,
  &:disabled:hover {
    color: #cfd8dc;
    border: 1px solid #cfd8dc;
    cursor: default;
    opacity: 1;
  }
`

const Button = props => {
  return (
    <StyledButton
      className={props.buttonClassName}
      disabled={props.disabled}
      onClick={props.onClick}
      {...props}
    >
      {props.children}
    </StyledButton>
  )
}

Button.propTypes = {
  children: PropTypes.node.isRequired,
  onClick: PropTypes.func,
  disabled: PropTypes.bool,
  buttonClassName: PropTypes.string
}

Button.defaultProps = {
  onClick: null,
  disabled: false,
  buttonClassName: ''
}

export default Button
