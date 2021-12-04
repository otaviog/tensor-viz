"""Tests the shader program.
"""

import unittest
from pathlib import Path

import tenviz


class TestProgram(unittest.TestCase):
    """Test shader program loading.
    """

    @staticmethod
    def test_load_fs():
        """Are the load functions working as expected.
        """
        context = tenviz.Context()
        with context.current():
            shader_dir = Path(tenviz.__file__).parent / "shaders"

            tenviz.load_program_fs(
                shader_dir / "phong.vert", shader_dir / "phong.frag")
