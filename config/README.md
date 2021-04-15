# How Configuration Is Organized

Configuration is divided into two classes:

1. **Global** configuration. This is configuration that applies universally to *all* Zippy instances. 
2. **Device-specific** configuration. This is configuration that applies to a single Zippy instance. Examples include 
camera calibrations.

## Global Configuration
Global configuration is stored in `config/global`

File Path | Proto | Owner | Description
--------- | ----- | ----- | -----------
`config/global/point_and_go_options.default.pbtext` | [applications/point_and_go/proto/point_and_go_options.proto](../applications/point_and_go/proto/point_and_go_options.proto) | @ianbaldwin |  
`config/global/estimator_options.default.pbtext` | [packages/estimation/proto/estimator_options.proto](../packages/estimation/proto/estimator_options.proto) | @ianbaldwin |
`config/global/executor_options.default.pbtext` | [packages/planning/proto/executor_options.proto](../packages/planning/proto/executor_options.proto) | @ianbaldwin |
`config/global/trajectory_options.default.pbtext` | [packages/planning/proto/trajectory_options.proto](../packages/planning/proto/trajectory_options.proto) | @ianbaldwin |
`config/global/planner_options.default.pbtext` | [packages/planning/proto/planner_options.proto](../packages/planning/proto/planner_options.proto) | @ianbaldwin |
`config/global/arc_planner_options.default.pbtext` | [packages/planning/proto/planner_options.proto](../packages/planning/proto/planner_options.proto) | @ianbaldwin |
`config/global/apriltag_config.default.pbtext` | [packages/perception/fiducials/proto/apriltag_config.proto](../packages/perception/fiducials/proto/apriltag_config.proto) | @avi-zippy |
`config/global/apriltag_detector_options.default.pbtext` | [packages/perception/fiducials/proto/apriltag_detector_options.proto](../packages/perception/fiducials/proto/apriltag_detector_options.proto) | @avi-zippy |

## Device-specific configuration
Device-specific configuration is stored in `config/<serial number>`

File Path | Proto | Owner | Description
--------- | ----- | ----- | -----------
`config/<serial number>/calibration.json` | [packages/calibration/proto/system_calibration.proto](../packages/calibration/proto/system_calibration.proto) | @ianbaldwin |
`config/<serial number>/docking_calibration.json` | [packages/docking/proto/vehicle_calibration.proto](../packages/docking/proto/vehicle_calibration.proto) | @byungsoo-zippy |
`config/<serial number>/hald.json` | [packages/hald/proto/device_config.proto](../packages/hald/proto/device_config.proto)| @chrisbroaddus |

## How configuration is validated
1. All configuration files are treated as required, always. Missing files are treated as an error. 
2. Any additional files / directories are treated as an error. If you want to add a new kind of configuration file, you 
will have to update the configuration linter's (//applications/configuration_linter) expectations as appropriate. 
3. We verify that each of these configuration files can at least be read without errors. It is possible to extend the 
linter with further, additional validations but we will do this as needed on a case-by-case basis. 


## Simulator
Stored in `config/simulator`

The simulator is a special case of device. It has its own configuration for specifying simulator settings, but also takes a camera calibration.

File Path | Proto | Owner | Description
--------- | ----- | ----- | -----------
`config/simulator/calibration.json` | [packages/calibration/proto/system_calibration.proto](../packages/calibration/proto/system_calibration.proto) | @ianbaldwin |
`config/simulator/calibration.json` | [packages/unity_simulator/proto/simulator_settings.proto](../packages/unity_simulator/proto/simulator_settings.proto) | @robertcastle |
