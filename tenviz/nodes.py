"""Out-of-the-box geometries.
"""
from pathlib import Path

import torch

from .program import DrawProgram
from .buffer import buffer_from_tensor
from ._ctenviz import (DrawMode, PolygonMode, MatPlaceholder)
from ._ctenviz import Scene as _Scene
from .geometry import compute_normals

_SHADER_DIR = Path(__file__).parent / "shaders"


class Scene(_Scene):
    """A simple composite of other draw program nodes.
    """

    def __init__(self, nodes=None):
        super(Scene, self).__init__()
        if nodes is not None:
            for node in nodes:
                self.add(node)
            self.nodes = nodes
        else:
            self.nodes = []


def create_mesh(verts, faces, normals=None, calc_normals=False, texcoords=None):
    """
    Creates a draw program to render Gourad shading meshes.

    Args:
        verts (:obj:`torch.Tensor`): Vertices [Nx3].

        faces (:obj:`torch.Tensor`): Face indices [MxK].

        normals (:obj:`torch.Tensor`, optional): Per vertices normals
         [Nx3].

        texcoords (:obj:`torch.Tensor`, optional): Per vertices
         texture coordinates [Nx2].

    Returns:
        :obj:`tenviz.program.DrawProgram`: The draw program.

    """

    draw_mode = DrawMode.Triangles
    if faces.size(1) == 4:
        draw_mode = DrawMode.Quads
    mesh = DrawProgram(draw_mode,
                       _SHADER_DIR / "phong.vert",
                       _SHADER_DIR / "phong.frag", ignore_missing=True)

    mesh['in_position'] = buffer_from_tensor(verts)
    if normals is None and calc_normals:
        normals = compute_normals(verts, faces)

    if normals is not None:
        mesh['in_normal'] = buffer_from_tensor(normals)

    if texcoords is not None:
        mesh['in_texcoord'] = buffer_from_tensor(texcoords)

    mesh['AmbientColor'] = torch.tensor(
        [0.6, 0.6, 0.6, 1.0], dtype=torch.float)
    mesh['DiffuseColor'] = torch.tensor(
        [0.7, 0.7, 0.7, 1.0], dtype=torch.float)
    mesh['SpecularColor'] = torch.tensor(
        [0.8, 0.8, 0.8, 1.0], dtype=torch.float)

    mesh['Lightpos'] = torch.tensor([0.0, 50.0, 0.0, 1.0], dtype=torch.float)
    mesh['SpecularExp'] = 127.0

    mesh['Modelview'] = MatPlaceholder.Modelview
    mesh['ProjModelview'] = MatPlaceholder.ProjectionModelview
    mesh['NormalModelview'] = MatPlaceholder.NormalModelview

    mesh.indices.from_tensor(faces)
    mesh.set_bounds(verts)

    return mesh


def create_mesh_from_geo(geo, calc_normals=False):
    """Shortcut for creating meshes from a geometry object.

    Args:

        geo (:obj:`torch.geometry.Geometry`): Geometry object.

    Returns:
        :obj:`tenviz.program.DrawProgram`: Draw program.

    """
    return create_mesh(geo.verts, geo.faces, geo.normals, calc_normals)


class PointCloud(DrawProgram):
    """Simple point cloud rendering.
    """

    def __init__(self, verts, colors=None, point_size=1):
        """Creates the vertice data.

        Args:

            verts (:obj:`torch.Tensor`): Points. Float [Nx3] tensor.

            colors (:obj:`torch.Tensor): Per point color. Uint8 [Nx3] tensor.

            point_Size (int): The point size.
        """

        super(PointCloud, self).__init__(
            DrawMode.Points,
            _SHADER_DIR / "point.vert",
            _SHADER_DIR / "point.frag")

        if colors is None:
            colors = torch.tensor([255, 255, 255], dtype=torch.uint8)

        colors = colors.view(-1, 3)
        if colors.size(0) == 1:
            colors = colors.repeat(verts.size(0), 1)
        self['in_position'] = buffer_from_tensor(verts.float())
        self['in_color'] = buffer_from_tensor(colors, normalize=True)
        self['ProjModelview'] = MatPlaceholder.ProjectionModelview
        self._transparency = 1.0
        self.set_bounds(verts)
        self.transparency = self._transparency
        self.style.point_size = point_size

    @property
    def transparency(self):
        """The alpha blending between [0, 1.0]
        """
        return self._transparency

    @transparency.setter
    def transparency(self, value):
        self._transparency = value
        # self['Transparency'] = torch.tensor([value])


def create_quiver(pos, vecs, colors):
    """Creates a quiver model, or a point cloud with arrows.

    Args:

        pos (:obj:`torch.Tensor`): Starting points. Float [Nx3] tensor.

        vecs (:obj:`torch.Tensor`): Per point vector. Float [Nx3] tensor.

        colors (:obj:`torch.Tensor`): Per point color. Uint8 [Nx3] tensor.

    Returns:
        :obj:`tenviz.program.DrawProgram`: The draw program.

    """
    end_pos = pos + vecs

    lines = torch.empty(pos.size(0)*2, 3)
    lines[::2, :] = pos
    lines[1::2, :] = end_pos

    line_colors = torch.empty(pos.size(0)*2, 3, dtype=colors.dtype)
    line_colors[::2, :] = colors
    line_colors[1::2, :] = colors

    draw = DrawProgram(DrawMode.Lines, _SHADER_DIR / "quiver.vert",
                       _SHADER_DIR / "quiver.frag")

    draw['in_position'] = lines
    draw['in_color'] = line_colors
    draw['ProjModelview'] = MatPlaceholder.ProjectionModelview

    draw.set_bounds(lines)

    return draw


def create_virtual_camera(proj, extrinsic, color=None, show_axis=True,
                          line_width=3.0):
    """Creates a simple symbolic geometry of a camera's frustum. Useful
    for debugging poses.

    Args:

        proj (:obj:`tenviz.projection.Projection`): Projection parameters.

        extrinsic (:obj:`tenviz.Tensor` or :obj:`numpy.ndarray`):
         Camera to world matrix. Float [4x4] tensor.

        color ((float, float, float)): Wireframe's color.

        show_axis (bool): Show a small camera's axis.

        line_width (float): Wireframe's line width.

    Returns:
        :obj:`tenviz.program.DrawProgram`: The draw program.
    """
    verts = torch.tensor([
        [0, 0, 0],                                    # 0
        [proj.left, proj.top, -proj.near],            # 1
        [proj.left, proj.bottom, -proj.near],         # 2
        [proj.right, proj.bottom, -proj.near],        # 3
        [proj.right, proj.top, -proj.near],           # 4
        [proj.far_left, proj.far_top, -proj.far],     # 5
        [proj.far_left, proj.far_bottom, -proj.far],  # 6
        [proj.far_right, proj.far_bottom, -proj.far],  # 7
        [proj.far_right, proj.far_top, -proj.far],    # 8
    ], dtype=torch.float)

    indices = torch.tensor([[0, 5], [0, 6], [0, 7], [0, 8],
                            [1, 2], [2, 3], [3, 4], [4, 1],
                            [5, 6], [6, 7], [7, 8], [8, 5],
                            [0, 9], [0, 10], [0, 11]],
                           dtype=torch.int)

    draw = DrawProgram(DrawMode.Lines, _SHADER_DIR / "default.vert",
                       _SHADER_DIR / "default.frag")

    if color is None:
        color = torch.tensor([0.6, 0.8, 0.75])
    else:
        color = torch.tensor(color).float()

    draw['in_position'] = verts
    draw['Color'] = color
    draw['ProjModelview'] = MatPlaceholder.ProjectionModelview

    draw.indices.from_tensor(indices)
    draw.set_bounds(verts)

    draw.style.line_width = float(line_width)
    draw.transform = extrinsic

    length = proj.near
    scene = [draw]

    if show_axis:
        axis = create_quiver(torch.zeros(3, 3),
                             torch.tensor([[1.0, 0.0, 0.0],
                                           [0.0, 1.0, 0.0],
                                           [0.0, 0.0, -1.0]])*length,
                             torch.tensor([[255, 0, 0],
                                           [0, 255, 0],
                                           [0, 0, 255]], dtype=torch.uint8))
        axis.transform = extrinsic
        scene.append(axis)

    return Scene(scene)


def create_axis_grid(start, end, steps, show_axis=True, grid_color=None):
    """Creates an axis grid on the X-Z plane.

    Args:

        start (float): Starting coordinate on the XZ plane.

        end (float): Ending coordinate on the XZ plane.

        steps (int): How many lines.

        show_axis (bool): Show an axis on the origin.

    Returns:
        :obj:`tenviz.program.DrawProgram`: The draw program.
    """

    # pylint: disable=invalid-name

    space = torch.linspace(float(start), float(
        end), int(steps), dtype=torch.float)

    if space.nelement() == 0:
        raise RuntimeError(
            "Empty points grid. "
            "Please review the start, end and steps parameters.")

    zs, xs = torch.meshgrid(space, space)
    ys = torch.zeros_like(xs)
    xz_grid = torch.stack([xs, ys, zs], 2)
    xz_grid = xz_grid.view(-1, 3)

    size = space.size(0)
    indices = torch.empty((size - 1) ** 2, 4, dtype=torch.int32)

    count = 0
    for i in range(size - 1):
        for j in range(size - 1):
            next_line = i * size + j
            indices[count] = torch.tensor(
                [j, next_line, next_line + 1, j + 1], dtype=torch.int32)
            count += 1

    draw = DrawProgram(DrawMode.Quads, _SHADER_DIR / "default.vert",
                       _SHADER_DIR / "default.frag")

    draw['in_position'] = xz_grid
    if grid_color is None:
        grid_color = torch.tensor([1.0, 1.0, 1.0])
    draw['Color'] = grid_color.float()
    draw['ProjModelview'] = MatPlaceholder.ProjectionModelview

    draw.indices.from_tensor(indices)

    draw.set_bounds(xz_grid.view(-1, 3))
    draw.style.polygon_mode = PolygonMode.Wireframe

    scene = [draw]
    if show_axis:
        axis_size = (end - start)/steps
        axis = create_quiver(torch.zeros(3, 3),
                             torch.tensor([[1.0, 0.0, 0.0],
                                           [0.0, 1.0, 0.0],
                                           [0.0, 0.0, 1.0]])*axis_size,
                             torch.tensor([[255, 0, 0],
                                           [0, 255, 0],
                                           [0, 0, 255]], dtype=torch.uint8))
        scene.append(axis)
    return Scene(scene)
