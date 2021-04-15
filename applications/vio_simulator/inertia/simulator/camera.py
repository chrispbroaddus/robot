import numpy as np
from rigidbody import pr, unpr, normalized, SE3
from inertia import Camera


class Frame(object):
    """
    Represents metadata for a simulated camera frame that links observations back to the
    ground truth landmark positions.
    """
    def __init__(self, observations, true_observations, track_ids, landmark_ids):
        self.observations = observations
        self.true_observations = true_observations
        self.track_ids = track_ids
        self.landmark_ids = landmark_ids


def sample_within_ball(radius, shape):
    """
    Sample from the ball of radius R centered at the origin.
    """
    return (np.random.random_sample(shape) * 2 - 1) * radius


def sample_within_shell(rmin, rmax, shape):
    """
    Sample a vector with length between rmin and rmax
    """
    x = normalized(np.random.randn(*shape))
    return x * np.random.uniform(low=rmin, high=rmax, size=shape[:-1]+(1,))


def take_uniform(xs, n):
    """
    Take n uniformly spaced items from xs
    """
    if n >= len(xs):
        return xs
    else:
        return [xs[int(i)] for i in np.linspace(0, len(xs) - 1, n).round()]


def generate_camera_matrix(image_width, image_height, fx=None, fy=None, cx=None, cy=None, **kwargs):
    """
    Construct a camera matrix for the specified image dimensions.
    """
    fx = fx or image_width / 2.
    fy = fy or image_height / 2.
    cx = cx or image_width / 2.
    cy = cy or image_height / 2.
    return np.array([[fx, 0, cx],
                     [0, fy, cy],
                     [0, 0, 1]], float)


def generate_camera(image_xlim, image_ylim, **kwargs):
    """
    Construct a camera object for the specified image dimensions
    """
    if np.isscalar(image_xlim):
        image_xlim = (0, image_xlim)
    if np.isscalar(image_ylim):
        image_ylim = (0, image_ylim)
    image_width = max(map(abs, image_xlim))
    image_height = max(map(abs, image_ylim))
    return Camera(xlim=image_xlim,
                  ylim=image_ylim,
                  matrix=generate_camera_matrix(image_width, image_height, **kwargs))


def generate_landmarks_around_trajectory(trajectory, duration, landmarks_per_second=500.,
                                         landmark_min_distance=.5, landmark_max_distance=20., **kwargs):
    """
    Sample landmarks within a tube of specified radius around the device trajectory.
    """
    num_landmarks = int(round(landmarks_per_second * duration))
    ts = np.linspace(0, duration, num_landmarks)
    ps = trajectory.position(ts)
    return ps + sample_within_shell(landmark_min_distance, landmark_max_distance, shape=(num_landmarks, 3))


def generate_landmarks_on_plane(trajectory, duration, landmarks_per_second=500., landmark_max_distance=20., **kwargs):
    num_landmarks = int(round(landmarks_per_second * duration))
    ts = np.linspace(0, duration, num_landmarks)
    ps = trajectory.position(ts)
    return ps + np.c_[
        sample_within_ball(landmark_max_distance, shape=(num_landmarks, 2)), np.ones(num_landmarks) * -1.5]


def generate_landmarks(state_func, duration, landmark_generator, **kwargs):
    if landmark_generator == 'cloud':
        landmarks = np.array(generate_landmarks_around_trajectory(state_func.trajectory, duration, **kwargs))
    elif landmark_generator == 'floor':
        landmarks = np.array(generate_landmarks_on_plane(state_func.trajectory, duration, **kwargs))
    else:
        raise ValueError('unrecognized landmark generator: %s' % landmark_generator)

    # Some generators use cartesian coords, others use homogeneous coords
    if landmarks.shape[-1] == 3:
        landmarks = unpr(landmarks)
    assert landmarks.ndim == 2
    assert landmarks.shape[-1] == 4

    return landmarks


def generate_feature_observations(state_func,
                                  landmarks,
                                  camera,
                                  frame_timestamps,
                                  feature_sigma,
                                  near_clip=.1,
                                  far_clip=50.,
                                  track_drop_probability=0.,
                                  features_per_frame=100,
                                  outlier_probability=0,
                                  device_to_camera_transform=None,
                                  **kwargs):
    landmarks = np.asarray(landmarks)
    assert landmarks.ndim == 2
    assert landmarks.shape[1] == 4

    if device_to_camera_transform is None:
        device_to_camera_transform = SE3.identity()

    rcalib, pcalib = device_to_camera_transform.rp
    image_tl, image_br = zip(camera.xlim, camera.ylim)

    frames = []
    next_track_id = 0
    cur_track_ids = -np.ones(len(landmarks), int)
    for i, t in enumerate(frame_timestamps):
        p = state_func.trajectory.position(t)
        r = state_func.trajectory.orientation(t)
        landmarks_imu = np.c_[np.dot(landmarks[:, :3] - np.outer(landmarks[:, 3], p), r.T), landmarks[:, 3]]
        landmarks_local = np.c_[np.dot(landmarks_imu[:, :3] - np.outer(landmarks_imu[:, 3], pcalib), rcalib.T), landmarks_imu[:, 3]]
        projections = pr(np.dot(landmarks_local[:, :3], camera.matrix.T))
        xs = projections[:, 0]
        ys = projections[:, 1]

        # Determine which landmarks were tracked in the previous frame
        prev_mask = cur_track_ids >= 0

        # Determine which landmarks are between the near and far plane
        zs, ws = landmarks_local[:, 2], landmarks_local[:, 3]
        near_clip_mask = zs * np.sign(ws) >= near_clip * ws * np.sign(ws)
        far_clip_mask = zs * np.sign(ws) <= far_clip * ws * np.sign(ws)
        visible_mask = np.all((camera.xlim[0] <= xs, xs < camera.xlim[1],
                               camera.ylim[0] <= ys, ys < camera.ylim[1],
                               near_clip_mask, far_clip_mask), axis=0)

        # Drop tracks with probability p
        drop_mask = np.random.rand(len(landmarks)) < track_drop_probability

        # Select some landmarks to continue
        continue_mask = np.logical_and(prev_mask, np.logical_and(visible_mask, ~drop_mask))
        num_to_create = features_per_frame - np.sum(continue_mask)

        # If too many landmarks being continued then drop some more
        if num_to_create < 0:
            overflow_indices = take_uniform(np.flatnonzero(continue_mask), -num_to_create)
            continue_mask[np.array(overflow_indices)] = False
            num_to_create = 0

        # Reset track id for tracks that we did not continue
        cur_track_ids[~continue_mask] = -1

        # Select some new landmarks to track
        create_candidate_mask = np.logical_and(~prev_mask, visible_mask)
        create_indices = np.array(take_uniform(np.flatnonzero(create_candidate_mask), num_to_create), int)
        create_mask = np.zeros(len(cur_track_ids), bool)
        create_mask[create_indices] = 1

        # Create new tracks ids for tracks created this frame
        cur_track_ids[create_indices] = np.arange(next_track_id, next_track_id + len(create_indices))
        next_track_id += len(create_indices)

        # Add noise to feature observations
        track_mask = np.logical_or(create_mask, continue_mask)
        true_observations = projections[track_mask].copy()
        noisy_observations = true_observations + np.random.randn(*true_observations.shape) * feature_sigma

        # Generate some outliers
        outlier_mask = np.random.rand(len(noisy_observations)) < outlier_probability
        num_outliers = int(np.sum(outlier_mask))
        if num_outliers > 0:
            noisy_observations[outlier_mask] = np.random.uniform(low=image_tl, high=image_br, size=(num_outliers, 2))

        # Add values to the track structure
        frames.append(Frame(
            observations=noisy_observations,
            true_observations=true_observations,
            track_ids=cur_track_ids[track_mask],
            landmark_ids=np.flatnonzero(track_mask)))

    # Run some sanity checks
    assert all(len(frame.track_ids) > 0 for frame in frames), 'there is a frame with zero tracks'

    return frames
