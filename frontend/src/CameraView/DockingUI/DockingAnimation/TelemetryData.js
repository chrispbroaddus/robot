import React from 'react'
import PropTypes from 'prop-types'
import styled from 'styled-components'

const LabelsContainer = styled.div`
  font-weight: 600;
  margin-left: 20px;
  margin-top: 20px;
  line-height: 8px;
  letter-spacing: 1px;
  font-size: 13px;
`
const Label = styled.p`color: white;`
const NumberLabel = styled.span`color: #fe7867;`

const TelemetryData = props => {
  const { angle, distanceX, distanceY } = props

  const pi = Math.PI
  const angleDeg = angle * (180 / pi)
  const eucDistance = Math.pow(
    Math.pow(distanceX, 2) + Math.pow(distanceY, 2),
    0.5
  )

  return (
    <LabelsContainer>
      <Label>
        Angle: <NumberLabel>{angleDeg.toFixed(1)}&deg;</NumberLabel>
      </Label>
      <Label>
        Distance: <NumberLabel>{eucDistance.toFixed(3)}m</NumberLabel>
      </Label>
    </LabelsContainer>
  )
}

TelemetryData.propTypes = {
  angle: PropTypes.number,
  distanceX: PropTypes.number,
  distanceY: PropTypes.number
}

TelemetryData.defaultProps = {
  angle: 0,
  distanceX: 0,
  distanceY: 0
}

export default TelemetryData
