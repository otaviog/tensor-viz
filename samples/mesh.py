#!/usr/bin/env python
"""
Demonstrates how to load and show a mesh.
"""

from pathlib import Path

import tenviz
import tenviz.io
import tenviz.geometry


def _main():
    context = tenviz.Context(640, 480)
    geo = tenviz.io.read_3dobject(
        Path(__file__).parent / "data/mesh/teapot.off")
    geo = geo.torch()
    geo.normals = tenviz.geometry.compute_normals(geo.verts, geo.faces)

    with context.current():
        scene = tenviz.nodes.create_mesh_from_geo(geo)

    viewer = context.viewer([scene])
    viewer.reset_view()
    while True:
        key = viewer.wait_key(1)
        if key < 0:
            break


if __name__ == '__main__':
    _main()
