import React from 'react'
import styled from 'styled-components'
import PropTypes from 'prop-types'

const MenuContainer = styled.div`
  position: absolute;
  top: ${props => (props.top ? `${props.top}px` : 0)};
  left: ${props => (props.left ? `${props.left}px` : 0)};
  z-index: 2000;

  border-radius: 7px;
  background-color: #000000;
  color: #fff;
  padding: 10px 0;
  transition: 0.2s opacity;
  box-shadow: 0 2px 4px 0 rgba(0, 0, 0, 0.5);
  opacity: ${props => (props.visible ? 1 : 0)};
  visibility: ${props => (props.visible ? 'visible' : 'hidden')};
`

const Item = styled.div`
  font-size: 12px;
  color: ${props => (props.selected ? '#fe7867' : '#fff')};
  padding: 5px 12px;
  min-width: 140px;
  transition: 0.2s;
  user-select: none;
  text-align: center;

  &:hover {
    cursor: pointer;
    background-color: #f5a623;
    color: white;
  }
`

class TurnInPlaceMenu extends React.PureComponent {
  static propTypes = {
    top: PropTypes.number,
    left: PropTypes.number,
    visible: PropTypes.bool,
    handleClick: PropTypes.func
  }

  static defaultProps = {
    top: 0,
    left: 0,
    visible: false,
    handleClick: () => {}
  }

  constructor(props) {
    super(props)
  }

  handleClickMenuItem = evt => {
    const { top, left, handleClick } = this.props

    evt.stopPropagation()
    handleClick(top, left)
  }

  render() {
    const { top, left, visible } = this.props
    return (
      <MenuContainer top={top} left={left} visible={visible}>
        <Item onClick={this.handleClickMenuItem}>turn in place</Item>
      </MenuContainer>
    )
  }
}

export default TurnInPlaceMenu
