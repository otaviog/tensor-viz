#!/usr/bin/env python
"""
Test drawing to images.
"""

from pathlib import Path

import torch
import numpy as np
from PIL import Image

import tenviz


def _main():
    context = tenviz.Context(640, 480)

    verts = torch.tensor([[-1, 1, 0],
                          [-1, -1, 0],
                          [1, -1, 0],
                          [1, 1, 0]], dtype=torch.float32)

    faces = torch.tensor([[0, 1, 2],
                          [2, 3, 0]], dtype=torch.int32)

    tex_coords = torch.tensor([[0, 1],
                               [0, 0],
                               [1, 0],
                               [1, 1]], dtype=torch.float32)

    img = torch.from_numpy(
        np.array(
            Image.open(str(Path(__file__).parent / 'data/images/tex1.png'))))

    with context.current():
        quad = tenviz.nodes.create_mesh(verts, faces, texcoords=tex_coords)
        texture = tenviz.tex_from_tensor(img)
        print(texture)
        quad['Tex'] = tenviz.tex_from_tensor(img)

    viewer = context.viewer([quad])
    viewer.reset_view()
    while True:
        key = viewer.wait_key(1)
        if key < 0:
            break


if __name__ == '__main__':
    _main()
