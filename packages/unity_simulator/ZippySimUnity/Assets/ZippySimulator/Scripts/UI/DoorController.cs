using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

namespace Zippy {

    public class DoorController : MonoBehaviour {

        public Rigidbody mainBoxBody;

        public Button frontUpperLeftDoorButton;
        public Rigidbody frontUpperLeftDoorRigidbody;

        public Button frontUpperRightDoorButton;
        public Rigidbody frontUpperRightDoorRigidbody;

        public Button frontLowerLeftDoorButton;
        public Rigidbody frontLowerLeftDoorRigidbody;

        public Button frontLowerRightDoorButton;
        public Rigidbody frontLowerRightDoorRigidbody;

        public Button rearUpperLeftDoorButton;
        public Rigidbody rearUpperLeftDoorRigidbody;

        public Button rearUpperRightDoorButton;
        public Rigidbody rearUpperRightDoorRigidbody;

        public Button rearLowerLeftDoorButton;
        public Rigidbody rearLowerLeftDoorRigidbody;

        public Button rearLowerRightDoorButton;
        public Rigidbody rearLowerRightDoorRigidbody;

        enum DoorCommand {
            Close = 0,
            Open
        }
        enum DoorStatus {
            Middle = 0,
            FullyOpened,
            FullyClosed
        }

        private DoorCommand _frontUpperLeftDoorCommand;
        private DoorStatus _frontUpperLeftDoorStatus;
        private DoorCommand _frontUpperRightDoorCommand;
        private DoorStatus _frontUpperRightDoorStatus;

        private DoorCommand _frontLowerLeftDoorCommand;
        private DoorStatus _frontLowerLeftDoorStatus;
        private DoorCommand _frontLowerRightDoorCommand;
        private DoorStatus _frontLowerRightDoorStatus;

        private DoorCommand _rearUpperLeftDoorCommand;
        private DoorStatus _rearUpperLeftDoorStatus;
        private DoorCommand _rearUpperRightDoorCommand;
        private DoorStatus _rearUpperRightDoorStatus;

        private DoorCommand _rearLowerLeftDoorCommand;
        private DoorStatus _rearLowerLeftDoorStatus;
        private DoorCommand _rearLowerRightDoorCommand;
        private DoorStatus _rearLowerRightDoorStatus;

        void Start() {
            _frontUpperLeftDoorCommand = DoorCommand.Close;
            Button _frontUpperLeftDoorButton = frontUpperLeftDoorButton.GetComponent<Button>();
            _frontUpperLeftDoorButton.onClick.AddListener(FrontUpperLeftDoorCommandUpdater);
            _frontUpperRightDoorCommand = DoorCommand.Close;
            Button _frontUpperRightDoorButton = frontUpperRightDoorButton.GetComponent<Button>();
            _frontUpperRightDoorButton.onClick.AddListener(FrontUpperRightDoorCommandUpdater);
            _frontLowerLeftDoorCommand = DoorCommand.Close;
            Button _frontLowerLeftDoorButton = frontLowerLeftDoorButton.GetComponent<Button>();
            _frontLowerLeftDoorButton.onClick.AddListener(FrontLowerLeftDoorCommandUpdater);
            _frontLowerRightDoorCommand = DoorCommand.Close;
            Button _frontLowerRightDoorButton = frontLowerRightDoorButton.GetComponent<Button>();
            _frontLowerRightDoorButton.onClick.AddListener(FrontLowerRightDoorCommandUpdater);
            _rearUpperLeftDoorCommand = DoorCommand.Close;
            Button _rearUpperLeftDoorButton = rearUpperLeftDoorButton.GetComponent<Button>();
            _rearUpperLeftDoorButton.onClick.AddListener(RearUpperLeftDoorCommandUpdater);
            _rearUpperRightDoorCommand = DoorCommand.Close;
            Button _rearUpperRightDoorButton = rearUpperRightDoorButton.GetComponent<Button>();
            _rearUpperRightDoorButton.onClick.AddListener(RearUpperRightDoorCommandUpdater);
            _rearLowerLeftDoorCommand = DoorCommand.Close;
            Button _rearLowerLeftDoorButton = rearLowerLeftDoorButton.GetComponent<Button>();
            _rearLowerLeftDoorButton.onClick.AddListener(RearLowerLeftDoorCommandUpdater);
            _rearLowerRightDoorCommand = DoorCommand.Close;
            Button _rearLowerRightDoorButton = rearLowerRightDoorButton.GetComponent<Button>();
            _rearLowerRightDoorButton.onClick.AddListener(RearLowerRightDoorCommandUpdater);
        }

        private void Update() {
            updateDoorRotation(ref frontUpperLeftDoorRigidbody, ref _frontUpperLeftDoorStatus, _frontUpperLeftDoorCommand);
            updateDoorRotation(ref frontUpperRightDoorRigidbody, ref _frontUpperRightDoorStatus, _frontUpperRightDoorCommand);
            updateDoorRotation(ref frontLowerLeftDoorRigidbody, ref _frontLowerLeftDoorStatus, _frontLowerLeftDoorCommand);
            updateDoorRotation(ref frontLowerRightDoorRigidbody, ref _frontLowerRightDoorStatus, _frontLowerRightDoorCommand);
            updateDoorRotation(ref rearUpperLeftDoorRigidbody, ref _rearUpperLeftDoorStatus, _rearUpperLeftDoorCommand);
            updateDoorRotation(ref rearUpperRightDoorRigidbody, ref _rearUpperRightDoorStatus, _rearUpperRightDoorCommand);
            updateDoorRotation(ref rearLowerLeftDoorRigidbody, ref _rearLowerLeftDoorStatus, _rearLowerLeftDoorCommand);
            updateDoorRotation(ref rearLowerRightDoorRigidbody, ref _rearLowerRightDoorStatus, _rearLowerRightDoorCommand);
        }

        void revertDoorCommand(ref DoorCommand doorCommand) {
            switch (doorCommand) {
            case DoorCommand.Close:
                doorCommand = DoorCommand.Open;
                break;

            case DoorCommand.Open:
                doorCommand = DoorCommand.Close;
                break;
            }
        }

        void FrontUpperLeftDoorCommandUpdater() {
            revertDoorCommand (ref _frontUpperLeftDoorCommand);
        }

        void FrontUpperRightDoorCommandUpdater() {
            revertDoorCommand (ref _frontUpperRightDoorCommand);
        }

        void FrontLowerLeftDoorCommandUpdater() {
            revertDoorCommand (ref _frontLowerLeftDoorCommand);
        }

        void FrontLowerRightDoorCommandUpdater() {
            revertDoorCommand (ref _frontLowerRightDoorCommand);
        }

        void RearUpperLeftDoorCommandUpdater() {
            revertDoorCommand (ref _rearUpperLeftDoorCommand);
        }

        void RearUpperRightDoorCommandUpdater() {
            revertDoorCommand (ref _rearUpperRightDoorCommand);
        }

        void RearLowerLeftDoorCommandUpdater() {
            revertDoorCommand (ref _rearLowerLeftDoorCommand);
        }

        void RearLowerRightDoorCommandUpdater() {
            revertDoorCommand (ref _rearLowerRightDoorCommand);
        }

        /// <summary>
        /// Updates the door rotation.
        /// </summary>
        /// <returns>The door rotation.</returns>
        /// <param name="doorRigidbody">Door rigidbody.</param>
        /// <param name="_statusOpen">Status open.</param>
        private void updateDoorRotation(ref Rigidbody doorRigidbody, ref DoorStatus doorStatus, DoorCommand doorCommand) {
            if (doorStatus == DoorStatus.FullyClosed && doorCommand == DoorCommand.Close) {
                return;
            }

            if (doorStatus == DoorStatus.FullyOpened && doorCommand == DoorCommand.Open) {
                return;
            }

            var rotationDoorToMainBox = Quaternion.Inverse(mainBoxBody.rotation) * doorRigidbody.rotation;
            var angle = rotationDoorToMainBox.eulerAngles;

            switch (doorCommand) {
            case DoorCommand.Open: {
                Vector3 doorTopInBoxCoordinate = new Vector3(0f, 1f, 0f);
                var doorTop = rotationDoorToMainBox * doorTopInBoxCoordinate;

                if (doorTop [1] > 0) {
                    doorRigidbody.rotation *= Quaternion.Euler (1f, 0f, 0f);
                    doorStatus = DoorStatus.Middle;
                }
                else {
                    doorStatus = DoorStatus.FullyOpened;
                }
            }
            break;

            case DoorCommand.Close: {
                Vector3 doorTopInBoxCoordinate = new Vector3(0f, 0f, 1f);
                var doorTop = rotationDoorToMainBox * doorTopInBoxCoordinate;

                if (doorTop [1] < 0) {
                    doorRigidbody.rotation *= Quaternion.Euler (-1f, 0f, 0f);
                    doorStatus = DoorStatus.Middle;
                }
                else {
                    doorStatus = DoorStatus.FullyClosed;
                }
            }
            break;
            }
        }

    }
}
