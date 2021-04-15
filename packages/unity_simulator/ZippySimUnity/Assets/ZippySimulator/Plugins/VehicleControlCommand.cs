using System.Runtime.InteropServices;

namespace Zippy {
    [StructLayout(LayoutKind.Sequential)]
    public struct VehicleControlCommand {
        public float frontLeftWheelTorque;
        public float frontLeftWheelSteerAngle;

        public float frontRightWheelTorque;
        public float frontRightWheelSteerAngle;

        public float middleLeftWheelTorque;
        public float middleLeftWheelSteerAngle;

        public float middleRightWheelTorque;
        public float middleRightWheelSteerAngle;

        public float rearLeftWheelTorque;
        public float rearLeftWheelSteerAngle;

        public float rearRightWheelTorque;
        public float rearRightWheelSteerAngle;

        public float delta_left_rail_servo;
        public float delta_right_rail_servo;

        public float delta_left_slider_value;
        public float delta_right_slider_value;

    }
}