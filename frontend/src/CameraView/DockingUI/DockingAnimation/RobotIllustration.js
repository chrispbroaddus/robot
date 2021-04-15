import React from 'react'
import PropTypes from 'prop-types'
import styled from 'styled-components'

const Robot = styled.div`
  width: 70px;
  height: 70px;
  border: 1px solid white;
  border-radius: 10px;
  position: relative;
  transition: top 2s ease, left 2s ease, transform 1s ease;
  font-size: 25px;
  font-face: Montserrat-Light;
  line-height: 72px;
  color: white;
  text-align: center;

  ${props =>
    props.locationY &&
    props.locationX &&
    `
    top:  ${Math.floor(props.locationY)}px;
    left: ${Math.floor(props.locationX)}px;
    transform: rotate(${props.angle}deg);
    transform-origin: center;
  `};
`

const RobotIllustration = props => {
  const { location, angle } = props
  // change angle to degrees
  const pi = Math.PI
  const angleDeg = angle * (180 / pi)

  return (
    <Robot locationX={location.X} locationY={location.Y} angle={angleDeg}>
      {' '}+{' '}
    </Robot>
  )
}

RobotIllustration.propTypes = {
  locationX: PropTypes.number.isRequired,
  locationY: PropTypes.number.isRequired,
  angle: PropTypes.number.isRequired
}

RobotIllustration.propTypes = {
  location: PropTypes.shape({
    X: PropTypes.number.isRequired,
    Y: PropTypes.number.isRequired
  }).isRequired,
  angle: PropTypes.number.isRequired
}

export default RobotIllustration
