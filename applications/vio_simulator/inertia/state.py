import numpy as np
from mutablerecord import MutableRecordType, Required, InstanceOf, Length
from rigidbody import SO3, SE3


class InertialState(object, metaclass=MutableRecordType):
    """
    Represents an orientation, position, velocity, accel bias, and gyro bias.
    """
    pose = InstanceOf(SE3)
    velocity = Length(3)
    gyro_bias = Length(3)
    accel_bias = Length(3)

    class Atlas(object):
        """
        An atlas for the manifold of inertial states.
        """
        @classmethod
        def dof(cls, _):
            return InertialState.DoF

        @classmethod
        def perturb(cls, state, tangent):
            assert len(tangent) == InertialState.DoF
            return InertialState(gyro_bias=state.gyro_bias + tangent[3:6],
                                 accel_bias=state.accel_bias + tangent[9:12],
                                 velocity=state.velocity + tangent[6:9],
                                 pose=SE3.Atlas.perturb(state.pose, np.hstack((tangent[:3], tangent[12:]))))

        @classmethod
        def displacement(cls, x1, x2):
            pose_displacement = SE3.Atlas.displacement(x1.pose, x2.pose)
            return np.hstack((pose_displacement[:3],
                              x2.gyro_bias - x1.gyro_bias,
                              x2.velocity - x1.velocity,
                              x2.accel_bias - x1.accel_bias,
                              pose_displacement[3:]))

    @classmethod
    def from_array(cls, v):
        """
        Construct an inertial state from an array of 21 elements. The first 9 elements are interpreted
        as a rotation matrix in row major order. The next 12 elements are interpreted as gyro bias, velocity,
        accel bias, and position.
        """
        assert len(v) == 21
        v = np.asarray(v)
        return InertialState(gyro_bias=v[9:12],
                             accel_bias=v[15:18],
                             velocity=v[12:15],
                             pose=SE3(v[:9].reshape((3, 3)), v[18:]))

    @classmethod
    def from_tangent(cls, v):
        """
        Construct an inertial state from a vector in the tangent space at the origin.
        """
        return InertialState.Atlas.perturb(InertialState.origin(), v)

    @classmethod
    def origin(cls):
        """
        Construct the "zero" inertial state.
        """
        return InertialState(gyro_bias=np.zeros(3),
                             accel_bias=np.zeros(3),
                             velocity=np.zeros(3),
                             pose=SE3.identity())

    @property
    def position(self):
        return self.pose.position

    @position.setter
    def position(self, v):
        self.pose.position = v

    @property
    def orientation(self):
        return self.pose.orientation

    @orientation.setter
    def orientation(self, v):
        print("setting orientation to %s" % v)
        self.pose.orientation = v

    def flatten(self):
        """
        Get a representation of this state as an array of length 21.
        """
        return np.hstack((self.orientation.flatten(), self.gyro_bias, self.velocity, self.accel_bias, self.position))


# Do not make this a class member or else it will be picked up as a member by MutableRecord
InertialState.DoF = 15