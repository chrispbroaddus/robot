from mutablerecord import MutableRecordType, Required


class ImuMeasurement(object, metaclass=MutableRecordType):
    """
    Represents reading from a gyro and accelerometer.
    """
    timestamp = Required
    accel_reading = Required
    gyro_reading = Required


class Feature(object, metaclass=MutableRecordType):
    """
    Represents a feature observed in an image.
    """
    frame_id = Required
    position = Required


class Track(object, metaclass=MutableRecordType):
    """
    Represents a sequence of observations of a 3D point.
    """
    id = Required
    features = Feature.List()


class FeatureObservation(object, metaclass=MutableRecordType):
    """
    Represents a feature observation for a particular frame/landmark pair.
    """
    landmark_id = Required
    frame_id = Required
    feature = Required


class InertialSensorModel(object, metaclass=MutableRecordType):
    """
    Represents noise characteristics for an inertial measurement unit.
    """
    gyro_sigma = Required
    gyro_bias_sigma = Required
    accel_sigma = Required
    accel_bias_sigma = Required


class VisionSensorModel(object, metaclass=MutableRecordType):
    """
    Represents noise characteristics for a camera.
    """
    observation_sigma = Required


class Camera(object, metaclass=MutableRecordType):
    """
    Represents a camera matrix and viewport
    """
    matrix = Required
    xlim = Required
    ylim = Required
