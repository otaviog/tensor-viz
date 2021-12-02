"""Framebuffer functions.
"""
from tenviz import _ctenviz


def create_framebuffer(attach_map):
    """Create a framebuffer for handling multiple rendering
    buffers and a depth buffer.

    The returned object operates a dictionary that maps `int` to
    :obj:`tenviz.texture.Texture` objects. These int are the GLSL's
    output locations. For example:

    .. code-block:: python

       with ctx.current():
         fb = tenviz.create_framebuffer({0: tenviz.FramebufferTarget.RGBUint8,
                                         1: tenviz.FramebufferTarget.RFloat32})
       # call ctx.render(..., fb, ...)

       texture0 = fb[0]
       with ctx.current():
         image = texture0.to_tensor()
         depth_buffer = fb.get_depth().to_tensor()

    Args:

        attach_map (Dict[int, :obj:`tenviz.FramebufferTarget`]): The keys
         relates to GLSL's output locations. For example, in the GLSL
         line :code:`layout(location = 2) out vec3 rgb`, the
         `location` corresponds to `2`. The values are framebuffer
         rendering target formats.

    Returns:
        :obj:`tenviz._ctenviz.Framebuffer`: A Framebuffer.

    """
    fbuf = _ctenviz.create_framebuffer()
    for location, target in attach_map.items():
        fbuf.set_attachment(location, target)

    return fbuf
