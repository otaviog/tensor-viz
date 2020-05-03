#!/usr/bin/env python
"""
!!!Doesn't work since VAO can't be shared between GLFW windows.!!!
"""

from pathlib import Path

import numpy as np

import tenviz
import tenviz.io


def _main():
    ctx = tenviz.Context()

    geo = tenviz.io.read_3dobject(
        Path(__file__).parent / "data/mesh/teapot.off")
    with ctx.current():
        proj = tenviz.Projection.perspective(45.0, 0.5, 3.0, aspect=0.9)
        mtx = np.eye(4, dtype=np.float32)
        cam = tenviz.nodes.create_virtual_camera(proj, mtx)
        mesh = tenviz.nodes.create_mesh(
            geo.verts, geo.faces, calc_normals=True)
        grid = tenviz.nodes.create_axis_grid(-5, 5, 10)


    world_viewer = ctx.viewer([cam, mesh, grid],
                              tenviz.CameraManipulator.WASD)
    camera_viewer = ctx.viewer([mesh, grid],
                               tenviz.CameraManipulator.WASD)
    while True:
        # view_mtx = camera_viewer.view_matrix
        # view_mtx = np.linalg.inv(view_mtx)
        # cam.transform = view_mtx

        keys = [
            # world_viewer.wait_key(0),
            camera_viewer.wait_key(0)
        ]
        for key in keys:
            if key < 0:
                break


if __name__ == '__main__':
    _main()
