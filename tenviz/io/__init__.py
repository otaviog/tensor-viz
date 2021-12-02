"""3D data input and output utilities
"""
from pathlib import Path

import numpy as np
import stl

from tenviz.geometry import Geometry

from ._ply import read_ply, write_ply
from ._wavefront import read_obj
from ._off import read_off, write_off


def _read_stl(path):
    mesh = stl.mesh.Mesh.from_file(str(path))
    geo = Geometry(mesh.points.reshape(-1, 3))
    geo.faces = np.arange(geo.verts.shape[0]).reshape(-1, 3)

    return geo


def read_3dobject(path, nofaces=False):
    """Reads a 3D object file. Supported formats are: .off, .obj, .ply, and .stl.

    Args:

        (str or :obj:`pathlib.Path`): Object file path.

    Returns: (:obj:`tenviz.geometry.Geometry`): Loaded geometry.

    """

    path = Path(path)

    suffix = path.suffix.lower()

    if suffix == '.off':
        return read_off(path, nofaces).torch()

    if suffix == '.obj':
        return read_obj(path, nofaces).torch()

    if suffix == '.ply':
        return read_ply(path, nofaces).torch()

    if suffix == '.stl':
        return _read_stl(path).torch()

    raise RuntimeError("Unknown 3D object extension {}".format(path.suffix))


def write_3dobject(path, verts, faces=None, normals=None, colors=None):
    """Writes a 3D object file. Supported formats are: .off and .ply
    """
    # pylint: disable=unused-argument

    suffix = Path(path).suffix.lower()
    verts = verts.squeeze()
    if suffix == '.off':
        with open(str(path), 'w', encoding="ascii") as stream:
            write_off(stream, verts, faces)
    elif suffix == '.ply':
        write_ply(str(path), verts, faces, colors, normals)
    else:
        raise RuntimeError("Unknown 3D object extension {}".format(
            suffix))
