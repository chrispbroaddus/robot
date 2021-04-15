import React from 'react'
import PropTypes from 'prop-types'
import styled from 'styled-components'
import { VEHICLE_STATUS } from '../../redux/modules/vehicles'

const Container = styled.div`
  position: relative;
  padding: 0;
`

const Status = styled.div`
  position: absolute;
  z-index: 1;
  bottom: 10px;
  right: 12px;
  padding: 5px;
  font-size: 9px;
  line-height: 11px;
  text-align: center;
  text-transform: uppercase;
  border-radius: 2px;
  background-color: rgba(54, 54, 54, 0.5);
  color: ${props => props.color};
`

const Image = styled.img`
  width: 265px;
  position: relative;
  z-index: 0;
`

const ImageStatus = props => {
  let color
  let status
  if (props.status === VEHICLE_STATUS.INACTIVE) {
    color = '#fe7867'
    status = 'Inactive'
  } else if (props.status === VEHICLE_STATUS.ACTIVE) {
    color = '#50e3c2'
    status = VEHICLE_STATUS.ACTIVE
  } else if (
    props.status === VEHICLE_STATUS.OFFLINE ||
    props.status === 'launching'
  ) {
    color = '#f65045'
    status = 'Offline'
  } else if (props.status === VEHICLE_STATUS.NETWORK_DROP) {
    color = '#f65045'
    status = 'Unavailable'
  } else if (props.status === VEHICLE_STATUS.DISABLED) {
    color = 'lightslategray'
    status = 'Disabled'
  }

  return (
    <Container>
      <Status color={color}>
        {status}
      </Status>
      <Image src={props.src} alt="last vehicle frame" />
    </Container>
  )
}

ImageStatus.propTypes = {
  status: PropTypes.string.isRequired,
  src: PropTypes.string.isRequired,
  title: PropTypes.string
}

export default ImageStatus
