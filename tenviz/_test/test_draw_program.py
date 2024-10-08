"""Test the DrawProgram class
"""

import unittest
from pathlib import Path

import tenviz
import tenviz.io


class TestDrawProgram(unittest.TestCase):
    """Test draw programming.
    """

    @staticmethod
    def test_set_item():
        """Tests the use of draw program attributes.
        """
        context = tenviz.Context()
        geo = tenviz.io.read_obj(
            Path(__file__).parent / "../../samples/data/mesh/teapot.off").torch()

        with context.current():
            shader_dir = Path(tenviz.__file__).parent / "shaders"

            mesh = tenviz.DrawProgram(tenviz.DrawMode.Triangles,
                                      shader_dir / "phong.vert",
                                      shader_dir / "phong.frag")

            mesh['in_position'] = tenviz.buffer_from_tensor(geo.verts)
            # TODO: check if vert extists
            mesh['Modelview'] = tenviz.MatPlaceholder.Modelview
            mesh['ProjModelview'] = tenviz.MatPlaceholder.Projection
