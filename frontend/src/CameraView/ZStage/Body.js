import React from 'react'
import PropTypes from 'prop-types'
import styled from 'styled-components'

const SVG = styled.svg``

const Body = props => (
  <SVG
    width="191px"
    height="237px"
    viewBox="0 0 191 237"
    innerRef={props.innerRef}
    {...props}
  >
    <g
      id="Z-stage-UI"
      stroke="none"
      strokeWidth="1"
      fill="none"
      fillRule="evenodd"
    >
      <g id="z-stage-screen" transform="translate(-631.000000, -292.000000)">
        <g id="Group-Copy" transform="translate(506.000000, 229.000000)">
          <g id="Body" transform="translate(125.000000, 63.000000)">
            <path
              d="M183.965462,30.4432657 C188.219341,35.1982862 190.805779,41.4763992 190.805779,48.3586073 L190.805779,194.607652 L190.80578,194.607652 C190.80578,207.851164 183.026412,219.859298 170.939765,225.272578 C154.674103,232.557526 130.884152,236.2 99.5699136,236.2 C65.4166723,236.2 38.9009372,231.867121 20.0227082,223.201362 L20.0227086,223.201361 C8.08909805,217.723421 0.44,205.795736 0.44,192.664899 L0.44,48.3586073 L0.44,48.3586073 C0.44,41.4763992 3.02643824,35.1982862 7.28031723,30.4432657 C21.3518378,12.7152423 55.6030636,0.2 95.6228896,0.2 C135.642715,0.2 169.893941,12.7152423 183.965462,30.4432657 Z"
              id="Combined-Shape"
              fill="#B2B2B2"
            />
            <path
              d="M95.1565591,55.2564828 C135.59568,55.2564828 168.378065,57.9883112 168.378065,41.4041101 C168.378065,24.8199089 135.59568,11.3757576 95.1565591,11.3757576 C54.7174384,11.3757576 21.9350538,24.8199089 21.9350538,41.4041101 C21.9350538,57.9883112 54.7174384,55.2564828 95.1565591,55.2564828 Z"
              id="Oval-2"
              fill="#000000"
            />
            <ellipse
              id="Oval-5"
              fill="#B7FBFE"
              cx="71.24"
              cy="34.2"
              rx="6.8"
              ry="8.4"
            />
            <ellipse
              id="Oval-5-Copy"
              fill="#B7FBFE"
              cx="116.04"
              cy="34.2"
              rx="6.8"
              ry="8.4"
            />
            <g
              id="lines-copy"
              transform="translate(66.200000, 87.400000)"
              stroke="#F9F9F9"
              strokeWidth="1.6"
              strokeLinecap="square"
            >
              <path d="M0.344186047,14.4 L58.855814,14.4" id="Line-2-Copy" />
              <path d="M0.344186047,28 L58.855814,28" id="Line-2-Copy-2" />
              <path d="M0.344186047,41.6 L58.855814,41.6" id="Line-2-Copy-3" />
              <path d="M0.344186047,55.2 L58.855814,55.2" id="Line-2-Copy-4" />
              <path d="M0.344186047,68.8 L58.855814,68.8" id="Line-2-Copy-5" />
              <path d="M0.344186047,82.4 L58.855814,82.4" id="Line-2-Copy-6" />
              <path d="M0.344186047,96 L58.855814,96" id="Line-2-Copy-7" />
              <path
                d="M0.344186047,109.6 L58.855814,109.6"
                id="Line-2-Copy-8"
              />
              <path d="M0.344186047,0.8 L58.855814,0.8" id="Line-2" />
            </g>
          </g>
        </g>
      </g>
    </g>
  </SVG>
)

Body.propTypes = {
  innerRef: PropTypes.func
}
Body.defaultProps = {}

export default Body
