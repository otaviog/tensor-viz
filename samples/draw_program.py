#!/usr/bin/env python
"""
Demonstrates how to use the `DrawProgram` class.
"""

from pathlib import Path

import torch

import tenviz
import tenviz.io
import tenviz.geometry


def _main():
    context = tenviz.Context(640, 480)
    geo = tenviz.io.read_3dobject(
        Path(__file__).parent / "data/mesh/teapot.off").torch()

    geo.normals = tenviz.geometry.compute_normals(geo.verts, geo.faces)

    shader_dir = Path(tenviz.__file__).parent / "shaders"
    with context.current():
        mesh = tenviz.DrawProgram(tenviz.DrawMode.Triangles,
                                  shader_dir / "phong.vert",
                                  shader_dir / "phong.frag")

        mesh['in_position'] = geo.verts
        mesh['in_normal'] = geo.normals

        mesh['AmbientColor'] = torch.tensor([0.8, 0.6, 0.6, 1.0])
        mesh['DiffuseColor'] = torch.tensor([0.8, 0.6, 0.6, 1.0])
        mesh['SpecularColor'] = torch.tensor([0.8, 0.6, 0.6, 1.0])
        mesh['Lightpos'] = torch.tensor([0.0, 1.0, 0.0, 1.0])

        mesh['Modelview'] = tenviz.MatPlaceholder.Modelview
        mesh['ProjModelview'] = tenviz.MatPlaceholder.ProjectionModelview
        mesh['NormalModelview'] = tenviz.MatPlaceholder.NormalModelview

        mesh.indices.from_tensor(geo.faces)
        mesh.set_bounds(geo.verts)

    viewer = context.viewer([mesh], tenviz.CameraManipulator.WASD)
    viewer.reset_view()
    while True:
        key = viewer.wait_key(1)
        if key < 0:
            break


if __name__ == '__main__':
    _main()
