"""Test projection
"""

import unittest

import tenviz

# pylint: disable=missing-docstring


class TestProjection(unittest.TestCase):
    def test_from_sensor(self):
        """Test whatever if constructing a projection from the Blender's
        parameters is working.

        """
        proj = tenviz.Projection.from_blender_sensor(
            32, 18, 35.27039762259147, 395, 244, "HORIZONTAL", 0.1, 10)
        self.assertAlmostEqual(proj.left, -0.04536382211381167)
        self.assertAlmostEqual(proj.right, 0.04536382211381167)
        self.assertAlmostEqual(proj.bottom, -0.02802220910321531)
        self.assertAlmostEqual(proj.top, 0.02802220910321531)
        self.assertAlmostEqual(proj.near, 0.10000000149011612)
        self.assertAlmostEqual(proj.far, 10.0)

    def test_perspective(self):
        """Test the perspective projection.
        """
        proj = tenviz.Projection.perspective(45.0, 1.0, 5.0, aspect=0.888)
        self.assertEqual(45.0, proj.fov_y)
        self.assertEqual(1.0, proj.near)
        self.assertEqual(5.0, proj.far)
        self.assertEqual(0.888, proj.aspect)
        self.assertAlmostEqual(40.38922, proj.fov_x, delta=0.001)


if __name__ == '__name__':
    unittest.main()
