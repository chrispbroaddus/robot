import React from 'react'
import styled from 'styled-components'

const Dock = styled.div`
  width: 126px;
  height: 42px;
  border: 1px solid white;
  border-radius: 36px;
  position: relative;
  top: 0px;
  margin: auto;
  text-align: center;
  font-face: Monserrat-Light;
  font-size: 10px;
  color: white;
  line-height: 40px;
  letter-spacing: 1px;
`

const DockIllustration = props => {
  return <Dock>dock</Dock>
}

export default DockIllustration
