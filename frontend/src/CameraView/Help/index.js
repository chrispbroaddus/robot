import React from 'react'
import PropTypes from 'prop-types'
import Modal from 'react-modal'
import styled from 'styled-components'
import QuestionIcon from 'react-icons/lib/md/help-outline'

const Container = styled.div`
  position: absolute;
  bottom: 25px;
  left: 40px;
  z-index: 1100;
`

const HelpButton = styled.div`
  cursor: pointer;
  color: rgba(255, 255, 255, 0.6);
  text-shadow: 0 1px 2px rgba(0, 0, 0, 0.2);
  transition: 0.2s ease-in;
`

const ModalContainer = styled(Modal)`width: 700px;`
const ModalContents = styled.div`
  color: white;

  & h5 {
    font-family: 'Montserrat-Light', sans-serif;
    font-size: 11px;
    text-transform: uppercase;
    letter-spacing: 1px;
    margin: 0;
  }

  & hr {
    width: 100%;
    height: 0;
    border-bottom: 1px solid white;
    margin: 13px 0 20px 0;
  }

  & ul,
  & ol {
    margin: 0;
    padding: 0 0 0 25px;
  }

  & ol {
    list-style: none;
  }

  & li {
    font-size: 11px;
    letter-spacing: 1px;
    text-transform: uppercase;
    margin: 10px 0;
  }
`

const Key = styled.div`
  display: inline-block;
  min-width: 22px;
  color: #f65045;
  border: 1px solid #f65045;
  margin: 0 5px;
  padding: 5px;
  text-transform: uppercase;
  text-align: center;
`

const ColumnContainer = styled.div`
  display: flex;
  flex-direction: row nowrap;
`
const Column = styled.div``

class Help extends React.Component {
  static propTypes = {
    onClick: PropTypes.func.isRequired,
    onRequestClose: PropTypes.func.isRequired,
    isOpen: PropTypes.bool.isRequired
  }

  static defaultProps = {}

  constructor(props) {
    super(props)

    this.state = {
      modalIsOpen: false
    }
  }

  render() {
    const { onRequestClose, isOpen, onClick } = this.props

    return (
      <Container>
        <HelpButton active={isOpen} onClick={onClick}>
          <QuestionIcon size="30" />
        </HelpButton>
        <ModalContainer
          isOpen={isOpen}
          onRequestClose={onRequestClose}
          overlayClassName="modal-overlay"
          contentLabel="Help"
        >
          <ModalContents>
            <h5>Keyboard Shortcuts</h5>
            <hr />
            <ColumnContainer>
              <Column>
                <ol>
                  <li>
                    <Key>Shift</Key> + <Key>Click</Key> Set Exposure Point
                  </li>
                  <li>
                    <Key>Z</Key> Adjust Z-Stage
                  </li>
                  <li>
                    <Key>E</Key> Reset Exposure
                  </li>
                  <li>
                    <Key>S</Key> Stop Vehicle
                  </li>
                  <li>
                    <Key>C</Key> Command Override Mode
                  </li>
                </ol>
              </Column>
              <Column>
                <ol>
                  <li>
                    <Key>Shift</Key> + <Key>/</Key> Help mode
                  </li>
                  <li>
                    <Key>R</Key> Reverse Mode / Toggle Cameras
                  </li>
                  <li>
                    <Key>O</Key> Distortion grid
                  </li>
                  <li>
                    <Key>L</Key> Ground plane
                  </li>
                  <li>
                    <Key>M</Key> Toggle Map
                  </li>
                  <li>
                    <Key>B</Key> Toggle Bounding Boxes
                  </li>
                  <li>
                    <Key>3</Key> Toggle 3D Bounding Boxes
                  </li>
                  <li>
                    <Key>4</Key> Toggle Convex Hulls in 3D Bounding Boxes
                  </li>
                </ol>
              </Column>
            </ColumnContainer>
          </ModalContents>
        </ModalContainer>
      </Container>
    )
  }
}

export default Help
