#!/usr/bin/env python
"""Test the geometry functionality.
"""

import unittest
import os

import torch

import tenviz
import tenviz.geometry as geo


class TestGeometry(unittest.TestCase):
    """
    Tests geometry functions
    """

    @classmethod
    def setUpClass(cls):
        cls.model_filepath = os.path.join(os.path.dirname(
            __file__), '../../samples/data/mesh/teapot.off')

    def test_normals(self):
        """"Test the normals computation"""
        normals = geo.compute_normals(torch.tensor([[0.0, 1.0, 5.0],
                                                    [-1.0, 0.0, 5.0],
                                                    [1.0, 0.0, 5.0]]),
                                      torch.tensor([[0, 1, 2]], dtype=torch.int32))
        self.assertEqual(normals.shape, (3, 3))

        torch.testing.assert_allclose(normals, torch.Tensor([[0, 0, 1.0],
                                                             [0, 0, 1.0],
                                                             [0, 0, 1.0]]))

        verts, faces = tenviz.io.read_3dobject(
            TestGeometry.model_filepath).torch()
        normals = geo.compute_normals(verts, faces)

        self.assertEqual(len(verts), len(normals))


if __name__ == '__main__':
    unittest.main()
