// TODO: Cleanup - This component is not in use
import React from 'react'
import styled from 'styled-components'
import PropTypes from 'prop-types'

const Container = styled.div`
  position: absolute;
  left: 45px;
  bottom: 85px;
  z-index: 1000;
  margin: auto;
`
const CommandList = styled.ul`
  margin: 0;
  padding: 0;
  list-style-type: none;

  & > li {
    opacity: ${props => (props.disable ? 0.7 : 1)};
  }
`

const CommandListItem = styled.li`
  margin-top: 30px;
  cursor: pointer;

  &.active-command .command-chiclet,
  &.active-command .command-text {
    color: #fe7867;
  }
  &.active-command .command-chiclet {
    border: 1px solid #fe7867;
  }
`

const CommandChiclet = styled.span.attrs({
  className: 'command-chiclet'
})`
  font-size: 12px;
  line-height: 12px;
  letter-spacing: 1px;
  color: white;
  border: 1px solid white;
  user-select: none;
  padding: 4px 6px;
  margin-right: 7px;
  text-align: center;
  border-radius: 2px;
`

const CommandText = styled.div.attrs({
  className: 'command-text'
})`
  display: inline-block;
  font-size: 10px;
  line-height: 10px;
  text-transform: uppercase;
  color: white;
  letter-spacing: 1px;
  margin-left: 7px;
  user-select: none;
`

const CommandDisplay = props => {
  const { mode, disabled, onClick } = props
  return (
    <Container>
      <CommandList disable={disabled}>
        <CommandListItem
          className={mode === 'arc' ? 'active-command' : ''}
          onClick={() => onClick('arc')}
        >
          <CommandChiclet>A</CommandChiclet>
          <CommandText>Arc mode</CommandText>
        </CommandListItem>
        <CommandListItem
          className={mode === 'pag' ? 'active-command' : ''}
          onClick={() => onClick('pag')}
        >
          <CommandChiclet>P</CommandChiclet>
          <CommandText>Point and Go mode</CommandText>
        </CommandListItem>
      </CommandList>
    </Container>
  )
}

CommandDisplay.propTypes = {
  mode: PropTypes.oneOf(['pag', 'arc']).isRequired,
  onClick: PropTypes.func,
  disabled: PropTypes.bool
}

CommandDisplay.defaultProps = {
  disabled: false,
  onClick: () => {}
}

export default CommandDisplay
