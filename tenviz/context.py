"""Graphics library context managenment.
"""

from contextlib import contextmanager
from ._ctenviz import (Context as _Context, CameraManipulator, Scene)


def _asure_scene(scene):
    if scene is None:
        RuntimeError("Scene is None")

    if isinstance(scene, list):
        scene_inst = Scene()
        for node in scene:
            scene_inst.add(node)
        return scene_inst

    return scene


class Context(_Context):
    """Rendering context. Use :func:`current` context manager to bind it to
    subsequent commands.

    """

    def __init__(self, width=640, height=480):
        """Initialize the render.

        Args:

            width (int): Output image width.
            height (int): Output image height.
        """
        # pylint: disable=useless-super-delegation
        super(Context, self).__init__(
            width, height)

    def render(self, projection, view, framebuffer,
               scene=None, width=-1, height=-1):
        """Off screen rendering to a framebuffer.

        Args:

            projection (:obj:`torch.Tensor` or :obj:`numpy.ndarray`):
             4x4 projection matrix.

            view (:obj:`torch.Tensor` or :obj:`numpy.ndarray`):
             4x4 camera's view matrix.

            framebuffer (:obj:`tenviz.Framebuffer`): Target
             framebuffer. The rendering will be write to its textures.

            scene (List[:obj:`tenviz.ANode`]): Target scene.

            width (int): Optional rendering width, override the
             current context width.

            height (int): Optional rendering height, override the
             current context height.

        """

        return super(Context, self).render(
            projection, view, framebuffer,
            _asure_scene(scene), width, height)

    def viewer(self, scene=None, cam_manip=CameraManipulator.TrackBall):
        """Creates a viewer window.

        Args:

            scene (List[:obj:`tenviz.ANode`]): Scene to show.

            cam_manip (:obj:`CameraManipulator`): Which kind of viewer camera manipulator.

        Returns:
            :obj:`tenviz._ctenviz.Viewer`: Viewer object.

        """
        scene = _asure_scene(scene)

        return super(Context, self).viewer(scene, cam_manip)

    def show(self, scene, cam_manip=CameraManipulator.TrackBall):
        """Utility function to show a viewer window without handling key waits.

        Args:

            scene (List[:obj:`tenviz.ANode`]): Scene to show.

            cam_manip (:obj:`tenviz.CameraManipulator`): Which kind of viewer camera manipulator.
        """
        viewer = self.viewer(scene, cam_manip)

        while True:
            key = viewer.wait_key(1)
            if key < 0:
                break

        viewer.release()

    @contextmanager
    def current(self):
        """Context manager for binding the context as current. It must be
        issued on operations that deals with graphics resources. Example:

        .. code-block:: python

           ...

           with context.current():
                tenviz.nodes.create_mesh(...)
                tenviz.create_framebuffer(...)
                tenviz.buffer_from_tensor(...)

        """

        self._make_current()
        try:
            yield self
        finally:
            self._detach_current()
