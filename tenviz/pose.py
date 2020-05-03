"""Camera pose utilities.
"""

import bisect

# pylint: disable=unused-import
from ._ctenviz import SO3, SE3, Pose


class PoseDict:
    """A collection of camera pose ordered by time. Use :obj:`__getitem__`
    to retrieve a pose at a given time. The pose will be interpolated
    if the exact time isn't on the dictionary.
    """

    def __init__(self):
        self.poses = {}

    def __getitem__(self, time):
        if not self.poses:
            raise KeyError("Empty pose dictionary")

        times = sorted(list(self.poses.keys()))
        idx = bisect.bisect(times, time)

        if idx == 0:
            return self.poses[times[idx]]

        if idx == len(self.poses):
            return self.poses[times[-1]]

        time1 = times[idx-1]
        time2 = times[idx]
        pose1 = self.poses[time1]
        pose2 = self.poses[time2]

        t = (time - time1)/(time2 - time1)

        return pose1.interpolate(pose2, t)

    def add(self, time, pose):
        """Add a new pose at a given time.

        Args:

            time (float): Time.

            pose (:obj:`Pose`): Pose.
        """

        self.poses[time] = pose

    def __setitem__(self, time, pose):
        self.add(time, pose)

    def items(self):
        """
        Returns:
            Iterator[(float, :obj:`Pose`)]: Time and pose generator.

        """
        return self.poses.items()

    def keys(self):
        """
        Returns:
            Iterator[float]: Times iterator.
        """
        return self.poses.keys()

    def get_final_time(self):
        """
        Returns:
            float: The largest time value.
        """
        return list(self.poses.keys())[-1]

    def __len__(self):
        return len(self.poses)

    def __str__(self):
        return str(self.poses)

    def __repr__(self):
        return repr(self.poses)
