#!/usr/bin/env python
"""Simple 
"""

from pathlib import Path

import torch

import tenviz
import tenviz.io


def _main():
    context = tenviz.Context(640, 480)
    geo = tenviz.io.read_3dobject(
        Path(__file__).parent / "data/mesh/teapot.off").torch()

    with context.current():
        colors = torch.rand(geo.verts.size(), dtype=torch.float)
        colors = (colors.abs()*255).byte()
        pcl = tenviz.nodes.PointCloud(geo.verts, colors)

    viewer = context.viewer([pcl])
    viewer.reset_view()

    while True:
        if viewer.wait_key(5) < 0:
            break


if __name__ == '__main__':
    _main()
