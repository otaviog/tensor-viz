"""Test texture.
"""
import unittest
from pathlib import Path

from PIL import Image
import numpy as np
import torch

import tenviz

_TEST_IMAGE = Path(__file__).parent / '../../samples/data/images/tex1.png'


class TestTexture(unittest.TestCase):
    """Test the texture handling.
    """
    @classmethod
    def setUpClass(cls):
        cls.context = tenviz.Context()
        print("Initializing torch CUDA")
        torch.rand(10, 10).to("cuda:0")
        print("Done")

    def _test_to_tensor_types(self, image, target, keep_device=True):
        with self.context.current():
            tex = tenviz.tex_from_tensor(image, target)
            image2 = tex.to_tensor(keep_device)

            self.assertEqual(keep_device, image2.is_cuda)
            torch.testing.assert_allclose(image, image2.to("cpu"))

            tex = tenviz.tex_from_tensor(image.int(), target)
            image2 = tex.to_tensor(keep_device)
            torch.testing.assert_allclose(image.int(), image2.to("cpu"))

            tex = tenviz.tex_from_tensor(image.float(), target)
            image2 = tex.to_tensor(keep_device)
            torch.testing.assert_allclose(image.float(), image2.to("cpu"))

    def test_texture_2d(self):
        """Test the texture 2D.
        """
        image = torch.from_numpy(np.array(Image.open(str(_TEST_IMAGE))))

        self._test_to_tensor_types(image, tenviz.TexTarget.k2D, keep_device=True)
        self._test_to_tensor_types(image, tenviz.TexTarget.Rectangle, keep_device=True)

        self._test_to_tensor_types(image, tenviz.TexTarget.k2D, keep_device=False)
        self._test_to_tensor_types(image, tenviz.TexTarget.Rectangle, keep_device=False)

        image = image[:, :, 0]
        self._test_to_tensor_types(image, tenviz.TexTarget.k2D, keep_device=True)
        self._test_to_tensor_types(image, tenviz.TexTarget.Rectangle, keep_device=True)

    def test_texture_3d(self):
        """Tests the 3D texture support.
        """
        torch.manual_seed(10)
        image = torch.rand((16, 16, 16))
        with self.context.current():
            tex = tenviz.tex_from_tensor(image, tenviz.TexTarget.k3D)
            image2 = tex.to_tensor()
            torch.testing.assert_allclose(image2, image)
