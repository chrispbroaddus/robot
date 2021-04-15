import React from 'react'
import styled from 'styled-components'
import PropTypes from 'prop-types'
import DropdownContainer from '../DropdownContainer'
import DropdownItem from '../DropdownItem'
import Caret from 'react-icons/lib/fa/caret-down'

const StyledDropdownContainer = styled.div`
  position: relative;
  display: flex;
  flex-flow: row nowrap;
  border: 1px solid #cfd8dc;
  color: #90a4ae;
  height: 32px;
  cursor: pointer;
  margin: 8px 0;
`
const StyledDropdownPlaceholder = styled.div`
  font-family: 'Montserrat-Light';
  font-size: 10px;
  letter-spacing: 1px;
  color: #90a4ae;
  line-height: 13px;
  padding: 10px;
  flex: 1 1 auto;
`
const StyledDropdownArrow = styled.div`
  align-self: flex-end;
  border-left: 1px solid #cfd8dc;
  flex: 0 0 30px;
  width: 30px;
  height: 32px;
  cursor: pointer;
  text-align: center;
  color: #90a4ae;
  line-height: 30px; // For vertical centering
`

class Dropdown extends React.Component {
  constructor(props) {
    super(props)

    this.state = {
      showOptions: false
    }
  }

  handleSelect = (title, value) => {
    this.setState(
      {
        selected: {
          title,
          value
        }
      },
      () => {
        this.props.onSelect(this.state.selected)
      }
    )
  }

  handleClick = () => {
    this.setState({ showOptions: !this.state.showOptions })
  }

  handleKeyDown = evt => {
    console.log('hit')
    if (evt.keyCode === 27) {
      console.log('escape hit')
    }
  }

  render() {
    const childrenWithProps = React.Children.map(this.props.children, child =>
      React.cloneElement(child, {
        handleSelect: this.handleSelect,
        selected:
          this.state.selected && this.state.selected.value === child.props.value
      })
    )

    return (
      <StyledDropdownContainer
        className={this.props.class}
        onClick={this.handleClick}
        onKeyDown={this.handleKeyDown}
        onKeyPress={this.handleKeyDown}
      >
        <StyledDropdownPlaceholder>
          {(this.state.selected && this.state.selected.title) ||
            this.props.placeholder}
        </StyledDropdownPlaceholder>
        <StyledDropdownArrow>
          <Caret size={8} />
        </StyledDropdownArrow>
        {this.state.showOptions && (
          <DropdownContainer
            onKeyDown={this.handleKeyDown}
            onKeyPress={this.handleKeyDown}
          >
            {childrenWithProps}
          </DropdownContainer>
        )}
      </StyledDropdownContainer>
    )
  }
}

Dropdown.propTypes = {
  children: PropTypes.node.isRequired,
  placeholder: PropTypes.string,
  class: PropTypes.string,
  onSelect: PropTypes.func
}

Dropdown.defaultProps = {
  placeholder: '',
  class: '',
  onSelect: () => {}
}

// This is done so we can have a single-line import for downstream components
// i.e. import { Dropdown, DropdownItem } from '../../Shared/Dropdown'
export { Dropdown, DropdownItem }
