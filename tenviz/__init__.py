"""TensorViz python bindings. Glues the C++ wrapper to the module's
interface.
"""

# pylint: disable=unused-import, no-name-in-module

import torch  # avoid THPVariableClass problem

from . import io
from . import geometry
from . import nodes

from .context import Context
from .program import load_program_fs, DrawProgram
from .projection import Projection
from .buffer import buffer_from_tensor, buffer_empty
from .texture import tex_from_tensor, tex_empty
from .framebuffer import create_framebuffer
from .viewer import take_screenshot
from ._ctenviz import (PolygonMode, PolygonOffsetMode, CameraManipulator,
                       MatPlaceholder, DrawMode, BufferTarget, BufferUsage,
                       DType, FramebufferTarget, TexTarget, Error)
