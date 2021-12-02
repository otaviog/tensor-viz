"""Wavefront loading
"""

import numpy as np

from tenviz.geometry import Geometry


def _next_line(file):
    for line in file:
        line = line.strip()
        if not line.startswith('#') and line != '':
            yield line


def read_obj(filepath, nofaces=False):
    """Read .obj file"""
    # pylint: disable=too-many-locals, too-many-branches

    verts = []
    obj_normals = []
    faces = []
    norm_faces = []

    with open(str(filepath), 'r', encoding="ascii") as file:
        for line in _next_line(file):
            line = line.split()

            if line[0] == 'v':
                verts.append((float(line[1]), float(line[2]), float(line[3])))

            if line[0] == 'vn':
                obj_normals.append(
                    (float(line[1]), float(line[2]), float(line[3])))

            if line[0] == 'f' and not nofaces:
                face = []
                norm_face = []
                for indices in line[1:]:
                    elem_idxs = indices.split('/')
                    if len(elem_idxs) >= 1:
                        face.append(int(elem_idxs[0])-1)

                    if len(elem_idxs) >= 3:
                        norm_face.append(int(elem_idxs[2])-1)

                faces.append(face)
                norm_faces.append(norm_face)

    normals = None
    if obj_normals:
        normals = np.empty((len(verts), 3), dtype=np.float32)
        for vface, nface in zip(faces, norm_faces):
            for vidx, nidx in zip(vface, nface):
                normals[vidx, :] = obj_normals[nidx]

    trig_faces = []
    for face in faces:
        if len(face) == 3:
            trig_faces.append(face)
            continue

        for i in range(1, len(face)-1):
            trig_faces.append([face[0], face[i], face[i+1]])

    return Geometry(np.array(verts, dtype=np.float32),
                    np.array(trig_faces, dtype=np.int32) if trig_faces else None,
                    normals=normals)
