""".PLY custom IO.
"""

import struct
from collections import defaultdict

import numpy as np

from tenviz.geometry import Geometry


class _PLYType:
    ply2struct = {'double': 'd', 'float': 'f', 'int': 'i',
                  'uchar': 'B', 'uint8': 'B',
                  'uint': 'I', 'uint32': 'I', 'uint16': 'H'}
    ply2py = {'double': float, 'float': float, 'int': int,
              'uchar': int, 'uint8': int,
              'uint': int, 'uint32': int, 'uint16': int}
    ply2np = {'double': np.float64, 'float': np.float32, 'int': np.int32,
              'uchar': np.uint8, 'uint8': np.uint8,
              'uint': np.uint32, 'uint32': np.uint32,
              'uint16': np.uint16}

    def __init__(self, type_name):
        try:
            self.struct = _PLYType.ply2struct[type_name]
            self.py_name = _PLYType.ply2py[type_name]
            self.np_name = _PLYType.ply2np[type_name]
        except KeyError:
            raise RuntimeError("Unknown ply type: {}".format(type_name))
        self.size = struct.calcsize(self.struct)

    def __str__(self):
        return str(self.np_name)

    def __repr__(self):
        return str(self)


class _PLYProperty:
    def __init__(self, name, type_name,
                 length_type_name=None, elem_type_name=None):
        self.name = name

        if type_name == 'list':
            self.val_type = _PLYType(elem_type_name)
            self.length_type = _PLYType(length_type_name)
        else:
            self.val_type = _PLYType(type_name)
            self.length_type = None

    def is_list(self):
        """Is this property a list type?

        """
        return self.length_type is not None

    def __str__(self):
        return ("Property [name = {}, val_type = {}, length_type = {}]".format(
            self.name, self.val_type, self.length_type))

    def __repr__(self):
        return str(self)


class _PLYElement:
    def __init__(self, name, length):
        self.name = name
        self.length = length
        self.properties = []

    def add_property(self, line):
        """Add a property to this element.

        Args:

            line (str): Header arg text line.

        """
        if line[1] == 'list':
            self.properties.append(_PLYProperty(
                line[4], 'list', line[2], line[3]))
        else:
            self.properties.append(_PLYProperty(line[2], line[1]))

    def get_struct(self, endianess):
        """Get the element reading struct.
        """
        fmt = endianess
        for prop in self.properties:
            if prop.is_list():
                return None
            fmt += prop.val_type.struct
        return struct.Struct(fmt)

    def __str__(self):
        return ("Element [name = {}, length = {}, props = {}]".format(
            self.name, self.length, self.properties))

    def __repr__(self):
        return str(self)


def _next_ply_line(file):
    while True:
        line = file.readline().decode().strip()
        if line != "comment":
            return line


def read_ply(path, nofaces):
    """Reads a ply file.

    Args:

        path (str or :obj:`pathlib.Path`): File path.

        nofaces (bool) : Don't load faces.

    Returns: (:obj:`tensorviz.geometry.Geometry`): Geometry.
    """
    # pylint: disable=too-many-branches

    path = str(path)
    elements = []

    with open(path, 'rb') as file:
        line = _next_ply_line(file)
        if line != "ply":
            raise RuntimeError("{}: header error".format(path))

        last_element = None

        while True:
            line = _next_ply_line(file).split()
            if line[0] == "end_header":
                start_offset = file.tell()
                break

            if line[0] == "element":
                last_element = _PLYElement(line[1], int(line[2]))
                elements.append(last_element)
            elif line[0] == "property":
                if last_element is None:
                    raise RuntimeError(
                        "Missformed ply file {}: {}".format(path, line))
                last_element.add_property(line)
            elif line[0] == "format":
                data_format = line[1]

    if not nofaces:
        required_elements = set(["vertex", "faces"])
    else:
        required_elements = set(["vertex"])

    if data_format == "ascii":
        data = _read_ascii_data(
            path, start_offset, elements, required_elements)
    elif data_format in ("binary_little_endian", "binary_big_endian"):
        data = _read_bin_data(path, start_offset, elements,
                              data_format, required_elements)
    else:
        raise RuntimeError("Unknown ply '{}' data format".format(data_format))

    vertex_data = data["vertex"]
    verts = np.hstack([
        np.array(vertex_data["x"], dtype=np.float32).reshape(-1, 1),
        np.array(vertex_data["y"], dtype=np.float32).reshape(-1, 1),
        np.array(vertex_data["z"], dtype=np.float32).reshape(-1, 1)])

    colors = None
    if "red" in vertex_data:
        colors = np.hstack([
            np.array(vertex_data["red"], dtype=np.uint8).reshape(-1, 1),
            np.array(vertex_data["green"], dtype=np.uint8).reshape(-1, 1),
            np.array(vertex_data["blue"], dtype=np.uint8).reshape(-1, 1)])

    normals = None
    if "nx" in vertex_data:
        normals = np.hstack([
            np.array(vertex_data["nx"], dtype=np.float32).reshape(-1, 1),
            np.array(vertex_data["ny"], dtype=np.float32).reshape(-1, 1),
            np.array(vertex_data["nz"], dtype=np.float32).reshape(-1, 1)])

    object_ids = None

    if "label" in vertex_data:
        object_ids = np.array(vertex_data["label"], dtype=np.int32).reshape(-1)

    faces = None
    if "face" in data:
        faces = np.array(data["face"]["vertex_indices"], dtype=np.int32)

        if "object_id" in data["face"]:
            object_ids = data["face"]["object_id"].reshape(-1).astype(np.int32)

    return Geometry(verts, faces, colors, normals, object_ids)


def _read_ascii_data(path, start_offset, elements, required_elements):
    with open(path, 'r') as file:
        file.seek(start_offset, 0)
        elem_dict = {}

        for elem in elements:
            prop_dict = defaultdict(list)

            for _ in range(elem.length):
                line = file.readline().strip().split()
                line_iter = iter(line)

                for prop in elem.properties:
                    if prop.is_list():
                        lst_size = prop.length_type.py_name(next(line_iter))
                        value = [prop.val_type.py_name(next(line_iter))
                                 for _ in range(lst_size)]
                        prop_dict[prop.name].append(value)
                    else:
                        prop_dict[prop.name].append(
                            prop.val_type.py_name(next(line_iter)))

            elem_dict[elem.name] = {key: np.array(data)
                                    for key, data in prop_dict.items()}

            if elem.name in required_elements:
                required_elements.remove(elem.name)
            if not required_elements:
                break

    return elem_dict


def _read_bin_data(path, start_offset, elements, data_format, required_elements):
    endian_char = '<'
    if data_format == "binary_big_engian":
        endian_char = '>'

    elem_dict = {}

    with open(path, 'rb') as file:
        file.seek(start_offset, 0)

        for elem in elements:
            fix_struct = elem.get_struct(endian_char)

            prop_dict = {prop.name:
                         np.empty((elem.length, 1),
                                  dtype=prop.val_type.np_name)
                         if not prop.is_list() else []
                         for prop in elem.properties}

            for i in range(elem.length):
                if fix_struct is not None:
                    values = fix_struct.unpack(file.read(fix_struct.size))

                    for value, prop in zip(values, elem.properties):
                        prop_dict[prop.name][i, 0] = value

                else:
                    for prop in elem.properties:
                        if prop.is_list():
                            list_size = struct.unpack(
                                '{}{}'.format(
                                    endian_char, prop.length_type.struct),
                                file.read(prop.length_type.size))[0]

                            lst_struct = struct.Struct('{}{}{}'.format(
                                endian_char, list_size, prop.val_type.struct))
                            lst_values = lst_struct.unpack(
                                file.read(lst_struct.size))

                            prop_dict[prop.name].append(lst_values)
                        else:
                            fmt = endian_char + prop.val_type.struct
                            value = struct.unpack(
                                fmt, file.read(prop.val_type.size))
                            prop_dict[prop.name][i, 0] = value[0]

            elem_dict[elem.name] = prop_dict

            if elem.name in required_elements:
                required_elements.remove(elem.name)
            if not required_elements:
                break

    return elem_dict


def write_ply(path, verts, faces, colors, normals):
    """Writes a geometry into a ply file.

    """
    with open(path, 'w') as file:
        file.write("ply\n")
        file.write("format binary_little_endian 1.0\n")
        file.write("element vertex {}\n".format(verts.shape[0]))
        file.write("property float x\n")
        file.write("property float y\n")
        file.write("property float z\n")
        if colors is not None:
            file.write("property uchar red\n")
            file.write("property uchar green\n")
            file.write("property uchar blue\n")
        if normals is not None:
            file.write("property float nx\n")
            file.write("property float ny\n")
            file.write("property float nz\n")
        if faces is not None:
            file.write("element face {}\n".format(faces.shape[0]))
            file.write("property list uchar int vertex_indices\n")
        file.write("end_header\n")

    with open(path, 'ab') as file:
        for idx in range(verts.shape[0]):
            point = verts[idx]
            file.write(struct.pack('<fff', point[0], point[1], point[2]))
            if colors is not None:
                col = colors[idx]
                file.write(struct.pack('<BBB', col[0], col[1], col[2]))
            if normals is not None:
                norm = normals[idx]
                file.write(struct.pack('<fff', norm[0], norm[1], norm[2]))

        if faces is not None:
            for face in faces:
                file.write(struct.pack('<Biii', len(face), *face))
