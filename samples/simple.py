#!/usr/bin/env python
"""Simplest example. Show how to create a DrawProgram and shows it.

"""

from pathlib import Path

import torch
import tenviz

geo = tenviz.io.read_3dobject(Path(__file__).parent / "data/mesh/teapot.off")
context = tenviz.Context()

SHADER_DIR = Path(__file__).parent / "shaders"

with context.current():
    program = tenviz.DrawProgram(tenviz.DrawMode.Points,
                                 vert_shader_file=SHADER_DIR / "simple.vert",
                                 frag_shader_file=SHADER_DIR / "simple.frag")
    # Set vertex attribute
    # geo.verts is torch tensor
    program["position"] = geo.verts

    # We also have place holders for current view matrices
    # passed by the user or from the viewer
    program["ProjModelview"] = tenviz.MatPlaceholder.ProjectionModelview

    # Set uniform
    program["color"] = torch.tensor(
        [0.8, 0.2, 0.2, 1.0], dtype=torch.float)

    program.style.point_size = 3

context.show([program], cam_manip=tenviz.CameraManipulator.WASD)
