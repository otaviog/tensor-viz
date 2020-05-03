#!/usr/bin/env python
"""Shows a mesh and its normals using a quiver node.
"""

from pathlib import Path

import torch

import tenviz
import tenviz.geometry


def main():
    context = tenviz.Context(640, 480)
    geo = tenviz.io.read_3dobject(
        Path(__file__).parent / "data/mesh/teapot.off").torch()
    geo.normals = tenviz.geometry.compute_normals(geo.verts, geo.faces)

    with context.current():
        mesh = tenviz.nodes.create_mesh_from_geo(geo)
        quiver = tenviz.nodes.create_quiver(
            geo.verts, geo.normals*0.05,
            torch.abs(geo.normals).clamp_min(0.0))

    viewer = context.viewer([mesh, quiver])
    while True:
        if viewer.wait_key(1) < 0:
            break


if __name__ == '__main__':
    main()
