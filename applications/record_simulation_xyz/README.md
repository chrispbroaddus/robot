# record_simulation_xyz

## What problem does this solve?
1. We record ground truth simulator point clouds (x, y, z w/r/t _camera_) for a single camera
2. We record ground truth vehicle pose (BaseLink / center of mass) for the vehicle

These results are placed into two separate files:

1. `${hostname}-${datetimestamp}-${cameraname}.xyz` contains `//packages/hal/proto/camera_sample.proto` instances with `type=PB_POINTCLOUD`
2. `${hostname}-${datetimestamp}-vehicle.pose` contains `//packages/hal/proto/unity_telemetry_envelop.proto` instances

In addition, we need the *effective* calibration used by the simulator. This should be in a file called `${hostname}-{datetimestamp}.calibration`


## How to run

 