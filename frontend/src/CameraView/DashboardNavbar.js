import React from 'react'
import PropTypes from 'prop-types'
import styled from 'styled-components'
import { Link } from 'react-router-dom'

import DashboardIcon from 'react-icons/lib/md/apps'

const Container = styled.div`
  position: absolute;
  top: 20px;
  right: 50px;
  z-index: 1000;
  color: white;

  & > a,
  & > span {
    color: white;
    text-decoration: none;
    padding-left: 5px;
    padding-right: 5px;
    font-size: 10px;
    font-weight: 400;
    letter-spacing: 1px;
    text-transform: uppercase;
    line-height: 16px;
    transition: opacity 1s;
  }
`

const IconContainer = styled(DashboardIcon)`
  margin-right: 5px;
  margin-bottom: 2px;
  width: 20px;
  height: 20px;
`

const DashboardLink = ({ className, children }) => (
  <Link to="/app/dashboard" className={className}>
    {children}
  </Link>
)

DashboardLink.propTypes = {
  className: PropTypes.string,
  children: PropTypes.node
}

DashboardLink.defaultProps = {
  className: '',
  children: null
}

const StyledDashboardLink = styled(DashboardLink)`
  cusor: pointer;
  &:hover {
    opacity: 0.5;
  }
`

const DashboardNavbar = props => {
  const { vehicleId } = props
  return (
    <Container>
      <StyledDashboardLink>
        <IconContainer>
          <DashboardIcon />
        </IconContainer>
        dashboard
      </StyledDashboardLink>{' '}
      | <span className="zippy-name">{vehicleId}</span>
    </Container>
  )
}

DashboardNavbar.propTypes = {
  vehicleId: PropTypes.string.isRequired
}
DashboardNavbar.defaultProps = {}

export default DashboardNavbar
