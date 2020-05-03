"""Tests object IO
"""

import unittest
from pathlib import Path

import numpy as np
import torch

import tenviz.io


class TestIO(unittest.TestCase):
    """3D objects IO.
    """

    def test_off(self):
        """Tests .off loading/writing
        """
        verts, faces = tenviz.io.read_3dobject(
            Path(__file__).parent /
            "../../samples/data/mesh/teapot.off")

        self.assertEqual(480, len(verts))
        # *2 because file use quads instead of trigs.
        self.assertEqual(880, len(faces))

        tenviz.io.write_3dobject(Path(__file__).parent / "mesh-off.out.off",
                                 verts, faces=faces)

        tenviz.io.write_3dobject(Path(__file__).parent / "pcl-off.out.off",
                                 verts)

    def test_ply_ascii(self):
        """Tests .ply loading/writing
        """
        verts, faces = tenviz.io.read_3dobject(
            Path(__file__).parent / "../../samples/data/mesh/teapot.ply")
        self.assertEqual(480, verts.shape[0])
        self.assertEqual(880, faces.shape[0])

    def test_ply_binary(self):
        """Tests .ply loading/writing
        """

        verts, _ = tenviz.io.read_3dobject(
            Path(__file__).parent /
            "../../samples/data/mesh/teapot.off")
        colors = (np.random.rand(verts.shape[0], 3)*255).astype(np.uint8)

        outpath = Path(__file__).parent / "pcl-ply.out.ply"
        tenviz.io.write_3dobject(outpath, verts, colors=colors)

        geo = tenviz.io.read_3dobject(outpath)

        self.assertEqual(verts.shape[0], geo.verts.shape[0])
        self.assertEqual(colors.shape[0], geo.colors.shape[0])

        torch.testing.assert_allclose(verts, geo.verts)
        torch.testing.assert_allclose(colors, geo.colors)

    def test_stl(self):
        """Tests .stl loading/writing
        """

        verts, faces = tenviz.io.read_3dobject(
            Path(__file__).parent / "../../samples/data/mesh/teapot.stl")

        self.assertEqual(2640, len(verts))
        self.assertEqual(880, len(faces))
