import React from 'react'
import styled from 'styled-components'
import PropTypes from 'prop-types'

const StyledDropdownItem = styled.div`
  height: 35px;
  border-bottom: 1px solid #f9f9f9;
  letter-spacing: 1px;
  box-sizing: border-box;
  color: #90a4ae;
  font-size: 9px;
  line-height: 35px;
  text-align: center;
  font-family: ${props =>
    props.selected === true ? 'Montserrat-Medium' : 'Montserrat-Light'};

  &:hover {
    background-color: rgba(0, 0, 0, 0.1);
    // font-family: 'Montserrat-Medium';
  }
`

const DropdownItem = props => {
  return (
    <StyledDropdownItem
      selected={props.selected}
      onClick={() => props.handleSelect(props.title, props.value)}
    >
      {props.children}
    </StyledDropdownItem>
  )
}

DropdownItem.propTypes = {
  value: PropTypes.oneOfType([PropTypes.string, PropTypes.number]).isRequired,
  title: PropTypes.oneOfType([PropTypes.string, PropTypes.number]).isRequired,
  children: PropTypes.node.isRequired,
  handleSelect: PropTypes.func,
  selected: PropTypes.bool
}

DropdownItem.defaultProps = {
  selected: false,
  handleSelect: () => {}
}

export default DropdownItem
