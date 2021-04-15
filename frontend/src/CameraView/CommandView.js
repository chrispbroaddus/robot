import React from 'react'
import PropTypes from 'prop-types'
import styled from 'styled-components'
import { bindActionCreators } from 'redux'
import { connect } from 'react-redux'
import * as commandDucks from '../redux/modules/commands'
import CommonShapes from '../Shared/CommonShapes'

const Container = styled.div`
  z-index: 900;
  margin: 10px 0;
  display: flex;
  flex-flow: row nowrap;
  align-items: center;
  position: relative;
  left: -7px;
`

const TextContainer = styled.div`
  position: relative;
  width: 300px;
  height: 30px;
  display: inline-block;
  margin-left: 5px;

  &.fadeOut {
    animation: 3s fadeOut forwards;
  }
`
const Text = styled.div`
  position: absolute;
  top: 0;
  left: 0;
  font-family: 'Montserrat', sans-serif;
  font-size: 8px;
  line-height: 30px;
  text-transform: uppercase;
  transition: 0.4s;
  user-select: none;
  cursor: default;
  opacity: ${props => (props.visible ? 1 : 0)};
  color: ${props => (props.color ? props.color : '#fff')}};
`

const SvgBall = styled.circle`
  fill: ${props => (props.color ? props.color : '#fff')}};
  ${props => {
    if (!!props.active === true) {
      return `filter: url(#blur-filter);`
    }
  }};
`

class CommandView extends React.PureComponent {
  static propTypes = {
    selected: PropTypes.number.isRequired,
    commands: PropTypes.arrayOf(CommonShapes.Command),
    lastCommand: CommonShapes.Command,
    timeout: PropTypes.number
  }

  static defaultProps = {
    lastCommand: null,
    commands: [],
    timeout: 1000
  }

  constructor(props) {
    super(props)
    this.state = {
      selected: 0,
      hidden: false
    }

    this.timeout = null
    this.showText = null
  }

  componentWillReceiveProps(nextProps) {
    if (
      nextProps.lastCommand &&
      this.props.lastCommand &&
      (nextProps.lastCommand.id !== this.props.lastCommand.id ||
        nextProps.lastCommand.type !== this.props.lastCommand.type ||
        nextProps.lastCommand.status !== this.props.lastCommand.status)
    ) {
      // If the next command is different, reset the timer
      clearTimeout(this.timeout)
      this.setState({ showText: true })
    }

    this.timeout = setTimeout(() => {
      this.setState({ showText: false })
    }, this.props.timeout)
  }

  componentDidUpdate(nextProps, nextState) {
    if (
      nextProps.lastCommand &&
      this.props.lastCommand &&
      (nextProps.lastCommand.id !== this.props.lastCommand.id ||
        nextProps.lastCommand.type !== this.props.lastCommand.type ||
        nextProps.lastCommand.status !== this.props.lastCommand.status)
    ) {
      // Only run the animation if the current command has changed
      if (this.svg) {
        this.svg.beginElement()
      }
    }
  }

  render() {
    const { showText } = this.state
    let command = this.props.lastCommand
    if (command === null) {
      command = {
        id: 'test',
        type: null,
        status: null
      }
    }
    const { type, status } = command

    let selected
    switch (status) {
      case Symbol.keyFor(commandDucks.COMMAND_STATUS.sent):
        selected = 2
        break
      case Symbol.keyFor(commandDucks.COMMAND_STATUS.executed):
        selected = 0
        break
      case Symbol.keyFor(commandDucks.COMMAND_STATUS.confirmed):
        selected = 1
        break
      default:
        selected = 0
        break
    }
    const commandText = commandDucks.commandTypeDecoder(type)

    return (
      <Container>
        <svg name="command-balls" height="30" width="30" viewBox="0 0 45 30">
          <filter id="blur-filter" x="-50%" y="-50%" width="200%" height="200%">
            <feGaussianBlur
              in="SourceGraphic"
              stdDeviation="5"
              result="blurOut"
            />
            <feBlend in="SourceGraphic" in2="blendOut" mode="normal" />
            <feMerge>
              <feMergeNode in="blendOut" />
              <feMergeNode in="SourceGraphic" />
            </feMerge>
          </filter>
          <g>
            <SvgBall
              cx="32"
              cy="15"
              r="6"
              color="#00e4b9"
              active={selected % 3 === 0}
            />
            <SvgBall
              cx="15"
              cy="5"
              r="6"
              color="#f5a623"
              active={selected % 3 === 1}
            />
            <SvgBall
              cx="15"
              cy="25"
              r="6"
              color="#F65045"
              active={selected % 3 === 2}
            />
            <animateTransform
              attributeName="transform"
              attributeType="XML"
              type="rotate"
              dur=".5s"
              calcMode="spline"
              keySplines="0.4 0 0.2 1; 0.4 0 0.2 1"
              repeatCount="1"
              begin="indefinite"
              fill="freeze"
              from={`${(selected - 1) % 3 * 120} 20 15`}
              to={`${selected % 3 * 120} 20 15`}
              ref={r => (this.svg = r)}
            />
          </g>
        </svg>
        <TextContainer className={!showText ? 'fadeOut' : ''}>
          <Text color="#00e4b9" visible={selected % 3 === 0}>
            {commandText} Executed
          </Text>
          <Text color="#f5a623" visible={selected % 3 === 1}>
            {commandText} Confirmed
          </Text>
          <Text color="#F65045" visible={selected % 3 === 2}>
            Sending {commandText}
          </Text>
        </TextContainer>
      </Container>
    )
  }
}

const mapStateToProps = (state, ownProps) => {
  const last = state.commands.slice(-1)[0]
  const lastCommand = last ? last : null

  return {
    commands: state.commands,
    lastCommand
  }
}

const mapDispatchToProps = dispatch => {
  return bindActionCreators({ ...commandDucks }, dispatch)
}

export default connect(mapStateToProps, mapDispatchToProps)(CommandView)
