import React from 'react'
import PropTypes from 'prop-types'
import styled, { keyframes } from 'styled-components'

const heightPx = 60

const AnimateIn = keyframes`
  from {
    height: 0;
    opacity: 0;
  }

  to {
    height: ${heightPx}px;
    opacity: 1;
  }
`

const AnimateOut = keyframes`
  from {
    height: ${heightPx}px;
    opacity: 1;
  }

  100% {
    height: 0;
    opacity: 0;
  }
`

const Container = styled.div`
  height: 0; // Initially hide the banner
  opacity: 0;
  width: 100%;
  background-color: #2b2b2b;
  // Requires a high z-index to be above the canvas and other conflicting
  // elements in the CameraView
  z-index: 1000;
  overflow: hidden;
  animation: 0.2s ease-in forwards
    ${props => (props.visible ? AnimateIn : AnimateOut)};
`

const InnerContainer = styled.div`
  padding-top: 22px;
  color: white;
  font-family: 'Montserrat-Regular';
  letter-spacing: 1px;
  font-size: 13px;
  text-align: center;
`

const BannerButton = styled.button`
  color: white;
  border: 1px solid white;
  padding: 5px 20px;
  border-radius: 5px;
  background: none;
  font-family: 'Montserrat-Light';
  letter-spacing: 1px;
  font-size: 10px;
  text-transform: uppercase;
  margin-left: 10px;

  &.active {
    border: 1px solid #f5a623;
    color: #f5a623;
    opacity: 0.5;
  }
`

const NotificationBanner = props => {
  const { message, data } = props.bannerMessage
  return (
    <Container className={props.className} visible={props.visible}>
      <InnerContainer>
        <span className="info">
          {data.length}{' '}
        </span>
        {message}
        {data.map((station, i) =>
          <BannerButton
            key={i}
            className={`banner-button ${props.active === station &&
            props.dockingMode
              ? 'active'
              : ''}`}
            onClick={() => props.onBannerButtonPress(station)}
          >
            {station}
          </BannerButton>
        )}
      </InnerContainer>
    </Container>
  )
}

NotificationBanner.propTypes = {
  active: PropTypes.number.isRequired,
  bannerMessage: PropTypes.shape({
    data: PropTypes.arrayOf(PropTypes.string).isRequired,
    message: PropTypes.string.isRequired
  }).isRequired,
  className: PropTypes.string,
  dockingMode: PropTypes.bool,
  visible: PropTypes.bool
}

export default NotificationBanner
