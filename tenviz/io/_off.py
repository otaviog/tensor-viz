""".off read/write functions
"""

import numpy as np

from tenviz.geometry import Geometry


def _next_off_line(file):
    while True:
        line = file.readline().strip()
        if line.startswith('#'):
            continue
        if line == '':
            continue

        break
    return line


def read_off(path, nofaces):
    """Implementation of .off file read.
    """
    # pylint: disable=too-many-locals

    with open(str(path), 'r', encoding="ascii") as file:
        header = file.readline().strip()
        if header != 'OFF':
            raise RuntimeError("File does not start with 'OFF'")

        line = _next_off_line(file)
        elems = line.split()
        num_verts = int(elems[0])
        if not nofaces:
            num_faces = int(elems[1])
        else:
            num_faces = 0

        verts = []
        for _ in range(num_verts):
            line = _next_off_line(file)
            elems = line.split()
            verts.append([float(elems[0]), float(elems[1]), float(elems[2])])

        faces = []
        for _ in range(num_faces):
            line = _next_off_line(file)
            elems = line.split()
            count = int(elems[0])

            if count == 3:
                faces.append([int(elems[1]),
                              int(elems[2]),
                              int(elems[3])])
            elif count == 4:
                idx1 = int(elems[1])
                idx2 = int(elems[2])
                idx3 = int(elems[3])
                idx4 = int(elems[4])

                faces.append([idx1, idx2, idx3])
                faces.append([idx3, idx4, idx1])
            else:
                # Assumes convex polygon:
                idxs = [int(e) for e in elems[1:]]

                for i in range(1, len(idxs)-1):
                    faces.append([idxs[0], idxs[i], idxs[i+1]])

        if len(faces) > 0:
            faces = np.array(faces, dtype=np.int32)
        else:
            faces = None

        return Geometry(np.array(verts, dtype=np.float32),
                        faces)


def write_off(stream, verts, faces):
    """Implementation of .off writing.

    """
    stream.write('OFF\n')

    face_cnt = 0
    if faces is not None:
        face_cnt = len(faces)

    stream.write('{} {} 0\n'.format(len(verts), face_cnt))

    for face0, face1, face2 in verts:
        stream.write('{} {} {}\n'.format(face0, face1, face2))

    if faces is None:
        return

    for face in faces:
        stream.write(str(len(face)))
        stream.write(' ')
        stream.write(' '.join(map(str, face.tolist())))
        stream.write('\n')
