import React from 'react'
import styled from 'styled-components'
import PropTypes from 'prop-types'

const Container = styled.div`margin: 10px;`

const Title = styled.span`
  color: #90a4ae;
  font-family: Montserrat-Light;
  font-size: 10px;
  letter-spacing: 0.5px;
  text-align: left;
`

const TitleCard = props => {
  return (
    <Container>
      <Title>
        {props.title}
      </Title>
    </Container>
  )
}

TitleCard.propTypes = {
  title: PropTypes.string.isRequired
}

export default TitleCard
