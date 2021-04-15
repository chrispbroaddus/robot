import React from 'react'
import PropTypes from 'prop-types'
import styled from 'styled-components'

const TextLogo = styled.span`
  z-index: 100;
  height: 44px;
  width: 120px;
  color: #f5a623;
  font-family: Montserrat-Medium;
  font-size: 36px;
  font-weight: 500;
  letter-spacing: 1px;
  line-height: 44px;

  ${props => (props.shadow ? 'text-shadow: 0px 2px 4px rgba(0,0,0,0.5);' : '')};
`

const Logo = props => {
  return (
    <TextLogo className={props.className} shadow={props.shadow}>
      zippy
    </TextLogo>
  )
}

Logo.propTypes = {
  shadow: PropTypes.bool,
  className: PropTypes.string
}

Logo.defaultProps = {
  shadow: false,
  className: ''
}

export default Logo
