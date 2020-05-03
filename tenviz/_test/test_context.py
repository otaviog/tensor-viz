"""Test the context class.
"""
import unittest

import torch
import numpy as np
import tenviz


class TestContext(unittest.TestCase):
    """Test the context class.
    """
    def test_make_current(self):
        """Is the context currenting working?
        """
        ctx = tenviz.Context()

        with ctx.current():
            with ctx.current():
                with ctx.current():
                    self.assertTrue(ctx.is_current())
                self.assertTrue(ctx.is_current())
            self.assertTrue(ctx.is_current())
        self.assertFalse(ctx.is_current())

    def test_attributes(self):
        """Are the attributes alright?
        """
        ctx = tenviz.Context()

        self.assertEqual(640, ctx.width)
        self.assertEqual(480, ctx.height)
        ctx.resize(512, 612)
        self.assertEqual(512, ctx.width)
        self.assertEqual(612, ctx.height)

        ctx.clear_color = np.array([1, 0, 1, 1])
        ctx.clear_color = torch.tensor([1, 0, 1, 1])

        self.assertEqual(1, int(ctx.clear_color[0]))
        self.assertEqual(0, int(ctx.clear_color[1]))
        self.assertEqual(1, int(ctx.clear_color[2]))
        self.assertEqual(1, int(ctx.clear_color[3]))

    def test_render(self):
        """Test rendering.
        """
        ctx = tenviz.Context(640, 480)

        with ctx.current():
            pcl = tenviz.nodes.PointCloud(torch.rand(100, 3))
            framebuffer = tenviz.create_framebuffer(
                {0: tenviz.FramebufferTarget.RGBAUint8})

        ctx.render(np.eye(4), np.eye(4), framebuffer, [pcl])
        self.assertEqual(640, framebuffer[0].width)
        self.assertEqual(480, framebuffer[0].height)

        ctx.render(torch.eye(4), torch.eye(4), framebuffer, [pcl],
                   width=320, height=240)
        self.assertEqual(320, framebuffer[0].width)
        self.assertEqual(240, framebuffer[0].height)

if __name__ == '__main__':
    unittest.main()
