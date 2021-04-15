import React from 'react'
import styled from 'styled-components'
import Logo from '../Shared/Logo'

const Navbar = styled.div`
  width: 100%;
  background-color: white;
  box-shadow: -1px 2px 8px 0 rgba(155, 155, 155, 0.2);
  padding: 20px 42px;
`

const Navigation = props => {
  return (
    <Navbar>
      <Logo />
    </Navbar>
  )
}

export default Navigation
