"""This module handles the creation of buffer objects instantiated by
the underling Graphics Library (GL). Current TensorViz is based only
on OpenGL.
"""

import tenviz._ctenviz as _ctenviz


class CudaMappedTensorContext:
    """Context manager for unmapping GPU memory rom tensors.
    """

    def __init__(self, mapped_tensor):
        self.mapped_tensor = mapped_tensor

    def __enter__(self):
        return self.mapped_tensor.tensor

    def __exit__(self, *args):
        self.mapped_tensor.unmap()


class Buffer(_ctenviz.Buffer):
    """An array buffer on the GL that can be mapped to tensor (as_tensor),
    or sliced as one. The slicing operator of this class can convert
    to/from GPU from/to PyTorch.

    """

    def __init__(self, target, usage):
        """Initialize the buffer. A context must be current (`tenviz.Context.current`).

        Args:

            target (:obj:`tenviz.BufferTarget`): Equivalent to OpenGL's
             GL_ARRAY_BUFFER or GL_ELEMENT_BUFFER.

            usage (:obj:`tenviz.BufferUsage`): Equivalent to OpenGL's
             buffer usage (DYNAMIC, STATIC...).
        """
        # pylint: disable=useless-super-delegation
        super().__init__(target, usage)

    def as_tensor(self):
        """Map the graphics buffer into a Cuda tensor. The returned tensor may
        be read and written. Shape changes don’t affect the GL buffer. The
        actual return is a context manager after the contest ends, the buffer
        is automatically unmapped back to the GL.

        A context must be current (:func:`tenviz.context.Context.current`).

        Example:

        .. code-block:: python

            with context.current():
                gl_buffer = tenviz.buffer_empty(10, 3)

                with gl_buffer.as_tensor() as tensor:
                    tensor[:, 3] = torch.rand(10, 3, torch.dtype=torch.float)

        Returns: 
            :obj:`CudaMappedTensorContext`: Mapped memory context manager.

        """

        return CudaMappedTensorContext(super().as_tensor())

    def as_tensor_(self):
        """Returns a mapped tensor without a context manager. Careful!
        """
        return super().as_tensor()


def buffer_from_tensor(tensor, target=_ctenviz.BufferTarget.Array,
                       usage=_ctenviz.BufferUsage.Dynamic, normalize=False, integer_attrib=False):
    """Creates a buffer from a tensor.

    Args:

        tensor (:obj:`torch.Tensor`): Tensor object. Maximum dim is 2.

        target (:obj:`tenviz.BufferTarget`): GL buffer target.

        usage (:obj:`tenviz.BufferUsage`): GL buffer usage.

        normalize (bool): Clamp between [0 1]?

        integer_attrib (bool): Bind as an intergers?

    Returns:
        :obj:`Buffer`: Allocated buffer.

    """
    buf = Buffer(target, usage)
    _ctenviz.register_resource(buf)

    buf.from_tensor(tensor)
    buf.normalize = normalize
    buf.integer_attrib = integer_attrib
    return buf


def buffer_empty(rows, cols=-1, btype=_ctenviz.DType.Float, target=_ctenviz.BufferTarget.Array,
                 usage=_ctenviz.BufferUsage.Dynamic, normalize=False, integer_attrib=False):
    """Creates an empty buffer.

    Args:

        rows (int): Buffer length.

        col (int, optional): If specified then the buffer is treated
         as array of vectors. Values must be 1, 2, 3 or 4.

        btype (:obj:`tenviz.DType`): Buffer's type.

        target (:obj:`tenviz.BufferTarget`): GL buffer target.

        usage (:obj:`tenviz.BufferUsage`): GL buffer usage.

        normalize (bool): If `False` then values aren't clamped between 0..1.0

        integer_attrib (bool): Should keep as integer values.

    Returns:
        :obj:`Buffer`: Allocated GL buffer.

    """
    buf = Buffer(target, usage)
    _ctenviz.register_resource(buf)

    buf.allocate(rows, cols, btype,)
    buf.normalize = normalize
    buf.integer_attrib = integer_attrib
    return buf
