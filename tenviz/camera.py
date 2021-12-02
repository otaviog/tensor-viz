"""Camera utilities.
"""

from tenviz import _ctenviz


def get_sphere_view(elevation, azimuth, center, radius, cam_params):
    """Returns a tight view matrix that encloses an entire sphere, as tightly as possible.

    Args:

        elevation (float): Elevation coordinates on the sphere's surface.

        azimuth (float): Azimuth coordinates on the sphere's surface.

        center ((float, float)): The sphere's center.

        radius (float): The sphere's radius.

        cam_params (:obj:`tenviz.Projection`): The camera projection parameters.

    Returns:
        :obj:`torch.Tensor`: A [4x4] view matrix.

    """
    distance = _ctenviz.get_min_bounding_distance(
        radius, min(cam_params.fov_y, cam_params.fov_x))

    return _ctenviz.get_sphere_view(elevation, azimuth, center, distance)
