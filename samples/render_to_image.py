#!/usr/bin/env python
"""
Test drawing to images.
"""

from pathlib import Path

import numpy as np
import matplotlib.pyplot as plt

import tenviz
import tenviz.geometry

_SHADER_DIR = Path(__file__).parent / "shaders"


def _main():
    context = tenviz.Context(512, 512)
    verts, faces = tenviz.io.read_3dobject(
        Path(__file__).parent / "data/mesh/teapot.off")

    with context.current():
        draw = tenviz.DrawProgram(
            tenviz.DrawMode.Triangles,
            vert_shader_file=_SHADER_DIR / 'multiple_out.vert',
            frag_shader_file=_SHADER_DIR / 'multiple_out.frag')
        draw['ProjModelview'] = tenviz.MatPlaceholder.ProjectionModelview
        draw['position'] = verts
        draw.indices.from_tensor(faces)

    proj = tenviz.Projection.perspective(
        45.0, 1.0, 10.0, aspect=1.0).to_matrix()

    cam = np.eye(4)
    cam[2, 3] = -2
    cam[0, 3] = -0.5

    context.clear_color = np.array([0, 0, 255, 0])

    with context.current():
        framebuffer = tenviz.create_framebuffer({
            0: tenviz.FramebufferTarget.RGBAUint8,
            1: tenviz.FramebufferTarget.RGBAFloat,
            2: tenviz.FramebufferTarget.RGBAInt32,
            3: tenviz.FramebufferTarget.RInt32})

    context.render(proj, cam, framebuffer, [draw])
    with context.current():
        plt.figure()
        plt.title("RGBA Uint8")
        image = framebuffer[0].to_tensor().cpu().numpy()
        plt.imshow(image[:, :, :3])
        print(image.max())

        plt.figure()
        plt.title("RGBA Float")
        image = framebuffer[1].to_tensor().cpu().numpy()
        plt.imshow(image[:, :, :3])
        print(image.max())

        plt.figure()
        plt.title("RGBA Int32")
        image = framebuffer[2].to_tensor().cpu().numpy()
        plt.imshow(image[:, :, :3]/255)
        print(image.max())

        plt.figure()
        plt.title("R Int32")
        image = framebuffer[3].to_tensor().cpu().numpy()
        plt.imshow(image[:, :, 0])
        print(image.max())

        plt.figure()
        plt.title("Depth")
        image = framebuffer.get_depth().to_tensor().cpu().numpy()
        plt.imshow(image[:, :])
        print(image.max())

    plt.show()


if __name__ == '__main__':
    _main()
