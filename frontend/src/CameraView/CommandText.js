import React from 'react'
import styled from 'styled-components'
import PropTypes from 'prop-types'

const Container = styled.div`
  z-index: 900;
  position: absolute;
  left: 0;
  bottom: 30px;
  width: 100%;
  line-height: 16px;
  text-align: center;
  font-size: 1em;

  display: ${props => (props.visible ? 'inline-block' : 'none ')}};
`

const Text = styled.span`
  color: white;
  display: ${props => (props.visible ? 'inline-block' : 'none ')}};
`

const HighlightedText = Text.extend`
  color: #fb5a45;
  padding: 10px;
  border-radius: 5px;
  background-color: rgba(0, 0, 0, 0.37);
  display: ${props => (props.visible ? 'inline-block' : 'none ')}};
`

const CommandText = props => {
  const { message, visible } = props
  return (
    <Container visible={visible}>
      <Text visible={!!message.main}>
        {message.main}
      </Text>
      <HighlightedText visible={!!message.command}>
        {message.command}
      </HighlightedText>
      <Text visible={!!message.rest}>
        {message.rest}
      </Text>
    </Container>
  )
}

CommandText.propTypes = {
  message: PropTypes.shape({
    main: PropTypes.string,
    command: PropTypes.string,
    rest: PropTypes.string
  }),
  visible: PropTypes.bool
}

CommandText.defaultProps = {
  message: {
    main: null,
    command: null,
    rest: null
  },
  visible: true
}

export default CommandText
