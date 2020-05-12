"""Build setup for TensorViz.
"""

import sys
import os
from setuptools import find_packages

try:
    from skbuild import setup
except ImportError:
    print('scikit-build is required to build from source.', file=sys.stderr)
    print('Please run:', file=sys.stderr)
    print('', file=sys.stderr)
    print('  python -m pip install scikit-build')
    sys.exit(1)

import torch
torch_root = os.path.dirname(torch.__file__)    

def _forbid_publish():
    argv = sys.argv
    blacklist = ['register', 'upload']

    for command in blacklist:
        if command in argv:
            values = {'command': command}
            print('Command "%(command)s" has been blacklisted, exiting...' %
                  values)
            sys.exit(2)


_forbid_publish()


REQUIREMENTS = [
    'numpy', 'numpy-stl', 'torch', 'pillow'
]

setup(
    name='TensorViz',
    version='0.0.1',
    author='Otavio Gomes',
    author_email='otavio.gomes@eldorado.org.br',
    zip_safe=False,
    description='OpenGL+PyTorch',
    packages=find_packages(exclude=['*._test']),
    install_requires=REQUIREMENTS,
    long_description='',
    include_package_data=True,
    package_data={'tenviz':
                  ['shaders/*.vert',
                   'shaders/*.frag',
                   'shaders/*.geo']})
