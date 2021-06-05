"""
Viewer handling
"""


import numpy as np

from .framebuffer import create_framebuffer
from ._ctenviz import FramebufferTarget


def take_screenshot(viewer) -> np.ndarray:
    """
    Returns the image of the current viewer.

    Args:
        viewer: The viewer object return by context

    Returns:
        An RGB image [HXWx3]
    """
    with viewer.context.current():
        framebuffer = create_framebuffer(
            {0: FramebufferTarget.RGBUint8})

        viewer.context.render(viewer.projection_matrix, viewer.view_matrix,
                              framebuffer, viewer.get_scene())
        screenshot = np.flipud(framebuffer[0].to_tensor().cpu().numpy())
    return screenshot
