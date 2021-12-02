"""Function and structures to load a shaders and draw stuff with them.
"""
from tenviz import _ctenviz


def load_program_fs(vert_shader=None, frag_shader=None, geo_shader=None):
    """Load a shader program from a file. Changes on the file are
    automatically reloaded while drawing.

    Args:

        vert_shader (str): Vertex shader file path. Errors are
         reported to the file `vert_shader`.out.

        frag_shader (str): Fragment shader file path. Errors are
         reported to the file `frag_shader`.out.

        geo_shader (str): Geometric shader file path. Errors are
         reported to the file `geo_shader`.out.

    Returns:
        :obj:`_ctenviz.DrawProgram`:A shader program object.

    """
    program = _ctenviz.load_program_fs(
        str(vert_shader) if vert_shader is not None else "",
        str(frag_shader) if frag_shader is not None else "",
        str(geo_shader) if geo_shader is not None else "")

    return program


class DrawProgram(_ctenviz.DrawProgram):
    """This class is the main part of TensorViz. It binds geometry to
    shader programs.  Program's attributes and uniforms can be set using
    the [] operator. It accepts int, floats, tensors, and texture objects
    for samplers.  Changes on the shader files triggers the automatic
    recompilation of the program. The errors are reported to files named
    with the `.out` suffix.

    Args:

        mode (:obj:`tenviz.DrawMode`): Drawing mode. Triangle, Quads...

        vert_shader_file (str): Vertex shader file path. Errors are
         reported to a file named `<vert_shader_file>`.out.

        frag_shader_file (str): Fragment shader file path. Errors are
         reported to the file `<frag_shader_file>`.out.

        geo_shader_file (str): Geometric shader file path. Errors are
         reported to the file `<geo_shader_file>`.out.

        program (optional): Already loaded program by
         `load_program_fs`. If specified, then the shader files
         are ignored.

        ignore_missing (bool): If set to `True` then missing shader
         uniform or attribute references will not raise an Error.
    """

    def __init__(self, mode, vert_shader_file=None, frag_shader_file=None,
                 geo_shader_file=None, program=None, ignore_missing=False):
        if program is None:
            program = load_program_fs(
                vert_shader_file, frag_shader_file, geo_shader_file)

        super().__init__(mode, program, ignore_missing)
