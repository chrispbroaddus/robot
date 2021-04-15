import configuration_items as ci
from applications.mercury.proto import system_description_pb2
from applications.mercury.proto import connections_pb2
from applications.point_and_go.proto import point_and_go_options_pb2
from applications.point_and_go.proto import docking_options_pb2
from packages.estimation.proto import estimator_options_pb2
from packages.hald.proto import device_config_pb2
from packages.executor.proto import executor_options_pb2
from packages.planning.proto import trajectory_options_pb2
from packages.planning.proto import trajectory_planner_options_pb2
from packages.calibration.proto import system_calibration_pb2
from packages.docking.proto import docking_station_pb2
from packages.docking.proto import vehicle_calibration_pb2
from packages.docking.proto import inverse_kinematic_docker_options_pb2
from packages.perception.fiducials.proto import apriltag_config_pb2
from packages.perception.fiducials.proto import apriltag_detector_options_pb2
from packages.teleop.proto import connection_options_pb2
from packages.machine_learning.proto import object_detection_runner_options_pb2
from packages.machine_learning.proto import object_volume_estimator_runner_options_pb2

from pathlib import Path

import os
import re


class ConfigurationChecker(object):
    """
    Top level class for configuration checking. Directory traversal gymnastics, etc. are done here whereas
    the fine-grain details of validating individual CIs is delegated to BasicConfigurationItem and its 
    children
    """

    def __init__(self, logger):
        self.logger = logger

        # If you want to remove an item, delete it from this array.
        # If you want to add an item, add it to this array. Objects are expected to be duck-compatible with
        # ConfigurationItem (which probably means you want to subclass ConfigurationItem and override validate
        # as needed)
        configItems = [
            ci.ProtobufConfigurationItem('mercury',
                                         'mercury.pbtxt',
                                         True,
                                         self.logger,
                                         system_description_pb2.SystemDescription()),
            ci.ProtobufJsonConfigurationItem('connections',
                                         'connections.json',
                                         False,
                                         self.logger,
                                         connections_pb2.Connections()),
            ci.ProtobufConfigurationItem('point_and_go',
                                         'point_and_go_options.default.pbtxt',
                                         True,
                                         self.logger,
                                         point_and_go_options_pb2.PointAndGoOptions()),
            ci.ProtobufConfigurationItem('docking_options',
                                         'docking_options.default.pbtxt',
                                         True,
                                         self.logger,
                                         docking_options_pb2.DockingOptions()),
            ci.ProtobufConfigurationItem('estimator',
                                         'estimator_options.default.pbtxt',
                                         True,
                                         self.logger,
                                         estimator_options_pb2.EstimatorOptions()),
            ci.ProtobufConfigurationItem('wheel_odometry',
                                         'wheel_odometry_options.default.pbtxt',
                                         True,
                                         self.logger,
                                         estimator_options_pb2.WheelOdometryOptions()),
            ci.ProtobufConfigurationItem('executor',
                                         'executor_options.default.pbtxt',
                                         True,
                                         self.logger,
                                         executor_options_pb2.ExecutorOptions()),
            ci.ProtobufConfigurationItem('trajectory',
                                         'trajectory_options.default.pbtxt',
                                         True,
                                         self.logger,
                                         trajectory_options_pb2.TrajectoryOptions()),
            ci.ProtobufConfigurationItem('trajectory_planner',
                                         'trajectory_planner_options.default.pbtxt',
                                         True,
                                         self.logger,
                                         trajectory_planner_options_pb2.TrajectoryPlannerOptions()),
            ci.ProtobufConfigurationItem('arc_planner',
                                         'arc_planner_options.default.pbtxt',
                                         True,
                                         self.logger,
                                         trajectory_planner_options_pb2.ArcPlannerOptions()),
            ci.ProtobufConfigurationItem('apriltag_config',
                                         'apriltag_config.default.pbtxt',
                                         True,
                                         self.logger,
                                         apriltag_config_pb2.AprilTagConfig()),
            ci.ProtobufConfigurationItem('apriltag_detector_options',
                                         'apriltag_detector_options.default.pbtxt',
                                         True,
                                         self.logger,
                                         apriltag_detector_options_pb2.AprilTagDetectorOptions()),
            ci.ProtobufConfigurationItem('inverse_kinematic_docker_options',
                                         'inverse_kinematic_docker_options.default.pbtxt',
                                         True,
                                         self.logger,
                                         inverse_kinematic_docker_options_pb2.InverseKinematicDockerOptions()),
            ci.ProtobufJsonConfigurationItem('docking_station_calibration',
                                             'docking_station_calibration.json',
                                             True,
                                             self.logger,
                                             docking_station_pb2.DockingStationList()),
            ci.ProtobufConfigurationItem('hald',
                                         'hald.conf',
                                         False,
                                         self.logger,
                                         device_config_pb2.DeviceConfig()),
            ci.ProtobufJsonConfigurationItem('system_calibration',
                                         'system_calibration.json',
                                         False,
                                         self.logger,
                                         system_calibration_pb2.SystemCalibration()),
            ci.ProtobufJsonConfigurationItem('docking_vehicle_calibration',
                                         'docking_vehicle_calibration.json',
                                         False,
                                         self.logger,
                                         vehicle_calibration_pb2.VehicleCalibration()),
            ci.ProtobufConfigurationItem('teleop',
                                         'teleop.pbtxt',
                                         True,
                                         self.logger,
                                         connection_options_pb2.ConnectionOptions()),
            ci.ProtobufConfigurationItem('object_detection_runner_options_front',
                                         'object_detection_runner_options.front_camera.pbtxt',
                                         True,
                                         self.logger,
                                         object_detection_runner_options_pb2.ObjectDetectionRunnerOptions()),
            ci.ProtobufConfigurationItem('object_detection_runner_options_rear',
                                         'object_detection_runner_options.rear_camera.pbtxt',
                                         True,
                                         self.logger,
                                         object_detection_runner_options_pb2.ObjectDetectionRunnerOptions()),
            ci.ProtobufConfigurationItem('object_volume_estimator_runner_options',
                                         'object_volume_estimator_runner_options.pbtxt',
                                         True,
                                         self.logger,
                                         object_volume_estimator_runner_options_pb2.ObjectVolumeEstimatorRunnerOptions()),
        ]

        ci_by_name = {x.name(): x for x in configItems}
        ci_by_file = {x.file(): x for x in configItems}

        # Make sure that people chose sane keys / file names that aren't duplicates
        if len(ci_by_name) != len(configItems):
            raise NameError('Detected duplicated name in configuration item list. Please fix this')

        if len(ci_by_file) != len(configItems):
            raise NameError('Detected duplicated file name in configuration item list. Please fix this')

        self.global_ci_by_file = {x.file(): x for x in configItems if x.is_global()}
        self.local_ci_by_file = {x.file(): x for x in configItems if not x.is_global()}

        # Regex used to recognize serial numbers. For now make this a GUID because why not
        self.serial_number_re = re.compile(
            r'[0-9A-Fa-f]{8}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{12}')

    def __check_dir(self, directory, items):
        """
        Do the actual heavy-lifting of verifying a configuration chunk. 
        :param directory: The directory whose contents we wish to verify
        :param items: A collection of ConfigurationItems. 
        :return: 
        """

        self.logger.info('Scanning [{}]'.format(str(directory)))

        found = {}

        for entry in os.scandir(os.fsencode(str(directory))):
            item = directory / os.fsdecode(entry.name)

            if item.is_file():
                if item.name in items:
                    items[item.name].validate(item)
                    found[item.name] = item.name
                else:
                    raise FileExistsError('Found unexpected file [{}]'.format(str(item)))
            else:
                raise FileExistsError('Found unexpected junk [{}]'.format(str(item)))

        missingFiles = items.keys() - found.keys()

        for x in missingFiles:
            self.logger.error('Missing [{}] inside directory [{}]'.format(x, str(directory)))

        if len(missingFiles) > 0:
            raise FileNotFoundError('Could not find one or more configuration items')

    def __check_global(self, directory):
        self.__check_dir(directory, self.global_ci_by_file)

    def __check_local(self, directory):
        self.__check_dir(directory, self.local_ci_by_file)

    def check(self, directory):
        """
        Start checking configuration contained in the tree rooted at directory. We guarantee, as a minimum, the following:
        1. All required files are present.
        2. No extra files are present.
        
        Depending on how things were configured in the constructor, we may do additional checks, which include loading
        protobuf files and verifying that they are at least syntactically reasonable. 
        
        If, at any point, we spot something astray, we throw and end processing immediately.
        :param directory: 
        :return: 
        """
        top = Path(directory)

        self.logger.info('Checking directory [{}]'.format(str(top)))
        if not top.exists():
            raise FileNotFoundError('Top level path [{}] does not exist'.format(str(top)))

        top = top.resolve()
        self.logger.info('Resolved [{}] --> [{}]'.format(directory, str(top)))

        checked_global = False

        for entry in os.scandir(os.fsencode(str(top))):
            item = top / os.fsdecode(entry.name)

            if item.is_dir():
                if 'global' == item.name:
                    self.logger.info('Checking global configuration in [{}]'.format(str(item)))
                    self.__check_global(item)
                    checked_global = True
                elif self.serial_number_re.match(item.name):
                    self.logger.info('Checking local configuration in [{}]'.format(str(item)))
                    self.__check_local(item)
                else:
                    # No extra crap
                    raise FileExistsError('Found an unexpected directory [{}]'.format(str(item)))
            elif item.is_file() and 'README.md' == item.name:
                # Ignore the readme
                pass
            else:
                # Again, no extra crap
                raise FileExistsError('Found unexpected junk [{}]'.format(str(item)))

        if not checked_global:
            raise FileNotFoundError('Did not find global configuration directory')
