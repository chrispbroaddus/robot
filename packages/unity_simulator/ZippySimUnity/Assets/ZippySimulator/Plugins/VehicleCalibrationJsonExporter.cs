using System.Collections;
using System.Collections.Generic;
using UnityEngine;

namespace Zippy {

    /// <summary>
    /// Vehicle calibration json exporter.
    /// </summary>
    public class VehicleCalibrationJsonExporter : MonoBehaviour {

        public HingeJoint hingeJointTorsoVsLeftRail;
        public HingeJoint hingeJointTorsoVsRightRail;
        public Camera fiducialDetectingCamera;
        public Transform vehicleAnchor;

        private void Start() {
            Quaternion quatVehicleAnchorWrtWorld;
            Vector3 translationVehicleAnchorWrtWorld;
            Geometry.InvertTransform (out quatVehicleAnchorWrtWorld, out translationVehicleAnchorWrtWorld, vehicleAnchor.rotation, vehicleAnchor.position);
            Quaternion quatVehicleAnchorWrtCamera;
            Vector3 translationVehicleAnchorWrtCamera;
            Geometry.ConcatenateTransform (out quatVehicleAnchorWrtCamera, out translationVehicleAnchorWrtCamera, quatVehicleAnchorWrtWorld, translationVehicleAnchorWrtWorld, fiducialDetectingCamera.transform.rotation, fiducialDetectingCamera.transform.position);
            Quaternion quatWorldWrtSlewAxisAfterRotate = hingeJointTorsoVsLeftRail.transform.rotation;
            Vector3 translationWorldWrtSlewAxisAfterRotate = (hingeJointTorsoVsLeftRail.transform.position + hingeJointTorsoVsRightRail.transform.position) / 2f;;
            translationWorldWrtSlewAxisAfterRotate.y += hingeJointTorsoVsLeftRail.anchor.y;
            Quaternion quatSlewAxisAfterRotateWrtWorld;
            Vector3 translationSlewAxisAfterRotateWrtWorld;
            Geometry.InvertTransform (out quatSlewAxisAfterRotateWrtWorld, out translationSlewAxisAfterRotateWrtWorld, quatWorldWrtSlewAxisAfterRotate, translationWorldWrtSlewAxisAfterRotate);
            Quaternion quatSlewAxisAfterRotateWrtVehicleAnchor;
            Vector3 translationSlewAxisAfterRotateWrtVehicleAnchor;
            Geometry.ConcatenateTransform (out quatSlewAxisAfterRotateWrtVehicleAnchor, out translationSlewAxisAfterRotateWrtVehicleAnchor, quatSlewAxisAfterRotateWrtWorld, translationSlewAxisAfterRotateWrtWorld, vehicleAnchor.rotation, vehicleAnchor.position);
            float distBetweenWheel = Mathf.Abs(hingeJointTorsoVsLeftRail.transform.position.x - hingeJointTorsoVsRightRail.transform.position.x);
            VehicleCalibrationJsonExporterWrapper.initialize("/tmp/zippy_simulator_vehicle_calibration.json",
                    translationVehicleAnchorWrtCamera.x, translationVehicleAnchorWrtCamera.y, translationVehicleAnchorWrtCamera.z, quatVehicleAnchorWrtCamera.x, quatVehicleAnchorWrtCamera.y, quatVehicleAnchorWrtCamera.z, quatVehicleAnchorWrtCamera.w,
                    translationSlewAxisAfterRotateWrtVehicleAnchor.x, translationSlewAxisAfterRotateWrtVehicleAnchor.y, translationSlewAxisAfterRotateWrtVehicleAnchor.z, quatSlewAxisAfterRotateWrtVehicleAnchor.x, quatSlewAxisAfterRotateWrtVehicleAnchor.y, quatSlewAxisAfterRotateWrtVehicleAnchor.z, quatSlewAxisAfterRotateWrtVehicleAnchor.w,
                    distBetweenWheel);
        }
    }
}
