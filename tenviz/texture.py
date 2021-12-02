"""Texture handling.
"""
from tenviz import _ctenviz


class Texture(_ctenviz.Texture):
    """Texture object.
    """

    def __init__(self, tex_target):
        # pylint: disable=useless-super-delegation
        super().__init__(tex_target)

    def __str__(self):
        return (f"{self.target} Texture of width {self.width}, "
                f"height {self.height} of type {self.dtype}")

    def __repr__(self):
        return str(self)


def tex_from_tensor(tensor, target=_ctenviz.TexTarget.k2D):
    """Creates a texture from a tensor. The texture type and format is
    mapped according to the passed target and the last size. For
    example, if target= :obj:`tenviz.TexTarget` and tensor.size() =
    [HxWx3], then a RGB texture with size [HxW].

    Args:

        tensor (:obj:`torch.Tensor`): Source tensor.

        target (:obj:`tenviz.TexTarget`): Texture target.

    Returns: (:obj:`Texture`): Created texture.
    """
    texture = Texture(target)
    _ctenviz.register_resource(texture)
    texture.from_tensor(tensor)

    return texture


def tex_empty(width, height, depth, channels=-1,
              btype=_ctenviz.DType.Float,
              target=_ctenviz.TexTarget.k2D):
    """Creates an empty texture.

    Args:

        width (int): Texture's width.

        height (int): Texture's height.

        depth (int): Texture's depth, or color channels if `target` is tenviz.TexTarget.k2D.

        channels (int): Channels for 3D textures (`target` is tenviz.TexTarget.k3D.

        btype (tenviz.DType): Buffer's type.

        target (:obj:`tenviz.TexTarget`): Texture's target (1D, 2D, 3D...).

    Returns: (:obj:`Texture`):
        Texture object.
    """
    texture = Texture(target)

    texture.empty(width, height, depth, channels, btype)
    return texture
