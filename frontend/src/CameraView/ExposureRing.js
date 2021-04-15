import React from 'react'
import PropTypes from 'prop-types'
import styled from 'styled-components'

const RingSVG = styled.svg``

const ExposureRing = props => {
  const { radius, onMouseDown, refFunc, active } = props
  return (
    <RingSVG
      name="exposure-ring"
      active={active}
      height={radius * 2}
      width={radius * 2}
      viewBox="0 0 200 200"
      innerRef={refFunc}
      onMouseDown={onMouseDown}
    >
      <defs>
        <linearGradient id="gradient" x1="0%" y1="0%" x2="80%" y2="80%">
          <stop offset="0%" stopColor="rgba(162,32,255,1)" />
          <stop offset="50%" stopColor="rgba(54,216,208,1)" />
          <stop offset="100%" stopColor="rgba(222,73,63,1)" />
        </linearGradient>
      </defs>

      <circle
        cx="100"
        cy="100"
        r="95"
        stroke="url(#gradient)"
        fill="none"
        strokeWidth="2"
      >
        <animateTransform
          attributeType="xml"
          attributeName="transform"
          type="rotate"
          from="0 100 100"
          to="-360 100 100"
          dur={active ? '3s' : '0s'}
          repeatCount="indefinite"
        />
      </circle>
      <circle
        cx="100"
        cy="100"
        r="55"
        stroke="#ccc"
        fill="none"
        strokeWidth="2"
        strokeDasharray="2, 6"
      >
        <animateTransform
          attributeType="xml"
          attributeName="transform"
          type="rotate"
          from="0 100 100"
          to="360 100 100"
          dur="10s"
          repeatCount="indefinite"
        />
      </circle>
    </RingSVG>
  )
}

ExposureRing.propTypes = {
  radius: PropTypes.number.isRequired,
  active: PropTypes.bool,
  onMouseDown: PropTypes.func,
  refFunc: PropTypes.func
}
ExposureRing.defaultProps = {
  active: false,
  onMouseDown: () => {},
  refFunc: () => {}
}

export default ExposureRing
