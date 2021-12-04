"""Pinhole camera projection"""

import math

import numpy as np

# pylint: disable=too-many-arguments, too-many-instance-attributes


class Projection:
    """Projection description.

    Attributes:

        left (float): Image X start.

        right (float): Image X end.

        bottom (float): Image Y end.

        top (float): Image Y start.

        near (float): near plane distance

        far (float): far plane distance

        far_left (float): Image X start on the far plane.

        far_right (float): Image X end on the far plane.

        far_top (float): Image Y start on the far plane.

        far_bottom (float): Image Y end on the far plane.

        fov_x (float): field of view in degrees.

        fov_y (float): field of view in degrees.

        aspect (float): aspect ration.
    """

    def __init__(self, left: float, right: float, bottom: float, top: float,
                 near: float, far: float):
        """Constructs a projection pyramid from near plane and far distance.
        """
        self.left = left
        self.right = right
        self.bottom = bottom
        self.top = top

        self.far_left = (left / near) * far
        self.far_right = (right / near) * far
        self.far_top = (top / near) * far
        self.far_bottom = (bottom / near) * far

        self.far = far
        self.near = near

        self.fov_x = np.rad2deg(math.atan(right / near)) * 2
        self.fov_y = np.rad2deg(math.atan(top / near)) * 2
        self.aspect = (right + abs(left)) / (top + abs(bottom))

    @classmethod
    def perspective(cls, fov_y: float, near: float, far: float,
                    aspect: float = None, fov_x: float = None):
        """Create a perspective projection.

        Args:

            fov: field of view angle in degrees.

            aspect: aspect ratio.

            near: near plane distance.

            far: far plane distance.

        Returns: :obj:`Projection`:
            Projection description. Use :func:`Projection.to_matrix` to get its matrix.
        """

        top = math.tan(np.deg2rad(fov_y / 2.0))*near
        if aspect is not None:
            right = top*aspect
        elif fov_x is not None:
            right = math.tan(np.deg2rad(fov_x / 2.0))*near
        else:
            right = top

        return cls(-right, right, -top, top, near, far)

    @classmethod
    def from_intrinsics(cls, kcam: np.ndarray, near: float, far: float):
        """Create a projetion description from a camera intrinsic matrix as
        used in 3D reconstruction.

        Args:
            kcam: 2x3 camera intrinsic matrix.

            near: near clipping plane distance.

            far: far clipping plane distance.

        Returns:
            :obj:`Projection`: Projection parameters.

        """
        right = (near*kcam[0, 2].item())/kcam[0, 0].item()
        top = (near*kcam[1, 2].item())/abs(kcam[1, 1].item())

        return cls(-right, right, -top, top, near, far)

    @classmethod
    def from_blender_sensor(cls, sensor_w: float, sensor_h: float, focal_len: float,
                            render_w: float, render_h: float,
                            sensor_fit: str, near: float, far: float):
        """Calculate the projection parameters from Blender's cameras. Useful
        for drawing Pix3D data.

        Calculation extracted from Goran Milovanovic answear on Stack
        Exchange: https://blender.stackexchange.com/a/16678

        Args:

            sensor_w: Sensor width in mm.
            sensor_h: Sensor height in mm.
            focal_len: Camera focal length in mm.
            render_w: Viewport width.
            render_h: Viewport height.
            sensor_fit: 'VERTICAL' or 'HORIZONTAL'.
            near: Near clipping plane distance.
            far: Far clipping plane distance.

        Returns:
            :obj:`Projection`: Projection parameters.

        """

        sensor_size = sensor_w
        view_factor = render_w
        if sensor_fit == "VERTICAL":
            sensor_size = sensor_h
            view_factor = render_h

        pixel_size = ((sensor_size*near)/focal_len)/view_factor

        right = 0.5*render_w*pixel_size
        top = 0.5*render_h*pixel_size

        return cls(-right, right, -top, top, near, far)

    def to_matrix(self) -> np.ndarray:
        """Creates a OpenGL-like projection matrix.

        Returns:
            A [4x4] projection matrix.
        """

        mtx = np.zeros((4, 4))
        mtx[0, 0] = 2.0 * self.near / (self.right - self.left)
        mtx[0, 1] = 0.0
        mtx[0, 2] = (self.right + self.left) / (self.right - self.left)
        mtx[0, 3] = 0.0

        mtx[1, 0] = 0.0
        mtx[1, 1] = 2.0 * self.near / (self.top - self.bottom)
        mtx[1, 2] = (self.top + self.bottom) / (self.top - self.bottom)
        mtx[1, 3] = 0

        mtx[2, 0] = 0.0
        mtx[2, 1] = 0.0
        mtx[2, 2] = -(self.far + self.near) / (self.far - self.near)
        mtx[2, 3] = -(2.0 * self.far * self.near) / (self.far - self.near)

        mtx[3, 0] = 0.0
        mtx[3, 1] = 0.0
        mtx[3, 2] = -1.0
        mtx[3, 3] = 0.0

        return mtx

    def __str__(self) -> str:
        return str(("Projection(left={}, right={}, bottom={}, top={}, near={}, far={}, "
                    "far_left={}, far_right={}, far_top={}, far_bottom={}, "
                    "fov_x={}, fov_y={}, aspect={})").format(
                        self.left, self.right, self.bottom, self.top, self.near, self.far,
                        self.far_left, self.far_right, self.far_top, self.far_bottom,
                        self.fov_x, self.fov_y, self.aspect))

    def __repr__(self) -> str:
        return str(self)
