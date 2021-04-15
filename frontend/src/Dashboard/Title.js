import React from 'react'
import styled from 'styled-components'
import ZippyImg from '../images/zippy.png'

const Container = styled.div`
  display: flex;
  flex-direction: row;
  align-items: center;
`

const ZippyLogo = styled.img`
  max-width: 69px;
  margin: 20px 40px;
`

const HeaderTitle = styled.h2`
  color: #90a4ae;
  font-family: 'Montserrat-Light';
  font-size: 18px;
`

const Title = props => {
  return (
    <Container>
      <ZippyLogo src={ZippyImg} alt="zippy-robot" />
      <HeaderTitle>Vehicle Dashboard</HeaderTitle>
    </Container>
  )
}

export default Title
