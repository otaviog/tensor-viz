"""
Geometry utilities.
"""

from enum import Enum

import torch


class Geometry:
    """Holds raw geometry data.

    Attributes:

        verts: Array of points or vertices. Numpy or torch array of
         shape [Nx3].

        faces: Array of indices to the vertices. Numpy or torch array
         of shape [MxK]. Triangle meshs have K equal to 3.

        colors: Array of per vertex colors. Numpy or torch array of
         shape [Nx3].

        normals: Array of per vertex normal. Numpy or torch array of
         shape [Nx3].
    """

    def __init__(self, verts, faces=None, colors=None, normals=None, object_ids=None):
        self.verts = verts
        self.faces = faces
        self.colors = colors
        self.normals = normals
        self.object_ids = object_ids

    def __str__(self):
        return "verts = {}, faces = {}, colors = {}, normals = {}".format(
            str(self.verts),
            str(self.faces),
            str(self.colors),
            str(self.normals))

    def __repr__(self):
        return str(self)

    def __iter__(self):
        yield self.verts
        yield self.faces

    def torch(self):
        """Converts the geometry from numpy to torch.

        Returns:
            :obj:`Geometry`: Geometry with its arrays being torch tensors.
        """

        def _to_torch(array):
            if not isinstance(array, torch.Tensor):
                return torch.from_numpy(array)
            return array

        geo = Geometry(_to_torch(self.verts))

        if self.faces is not None:
            geo.faces = _to_torch(self.faces)

        if self.colors is not None:
            geo.colors = _to_torch(self.colors)

        if self.normals is not None:
            geo.normals = _to_torch(self.normals)

        if self.object_ids is not None:
            geo.object_ids = _to_torch(self.object_ids)

        return geo


class NormalMethod(Enum):
    """Which method to use for computing normals.

    The only one that exists currently is the averaging face normals:
    https://www.khronos.org/opengl/wiki/Calculating_a_Surface_Normal
    """
    AVERAGE = 1


def compute_normals(verts, faces, method=NormalMethod.AVERAGE):
    """Computes the per vertex normal vector. Very slow.

    Args:

        verts (:obj:`torch.Tensor`): Vertice array. Float [Nx3] tensor.

        faces (:obj:`torch.Tensor`): Face array. Integer [Mx3] tensor.

        method (:obj:`NormalMethod`): Which method to compute the
         normals.

    Returns:
        :obj:`torch.Tensor`: Normal array. Float [Nx3] tensor.

    """
    # pylint: disable=unused-argument, invalid-name
    normals = torch.zeros(verts.size(0), 3)
    counts = torch.ones(verts.size(0))

    for face in faces:
        v0, v1, v2 = [verts[idx] for idx in face]

        tu = v1 - v0
        tu = tu/torch.norm(tu, 2)

        tv = v2 - v0
        tv = tv/torch.norm(tv, 2)

        normal = torch.cross(tu, tv)

        for idx in face:
            normals[idx] += normal
            counts[idx] += 1.0

    normals /= counts.view(-1, 1)

    normals /= torch.norm(normals, 2, dim=1).view(-1, 1)
    return normals


def to_triangles(faces):
    """Triangulation of convex polygons.

    Args:

        faces (:obj:`torch.Tensor`): Polygon's indices to vertex. This
         polygon must be convex to this method work. Integer [NxM] faces.

    Returns:

        :obj:`torch.Tensor`: Triangulated tensor with equal or
         greater N faces.

    """
    if faces.size(1) == 3:
        return faces

    trig_faces = []
    for face in faces:
        idxs = [int(idx) for idx in face]

        for i in range(1, len(idxs) - 1):
            trig_faces.append([idxs[0], idxs[i], idxs[i+1]])

    return torch.Tensor(trig_faces, dtype=torch.int32)
