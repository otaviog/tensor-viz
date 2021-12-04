# TensorViz: Prototype GLSL shaders with PyTorch

TensorViz is a module designed for prototyping GLSL shaders for Computer Vision applications.
It is integrated with PyTorch to make array/element buffers, framebuffers, or textures easily converted to/from tensors to improve testing and debugging.
For example, given the following shaders:

**Vertex shader:**


```glsl
#version 420

in vec4 position;
uniform mat4 ProjModelview;

void main() {
  gl_Position = ProjModelview * position;
}
```

**Fragment shader:**

```glsl
#version 420

out vec4 frag_color;

uniform vec4 color;

void main() {
	 frag_color = color;
}
```

Then run this with TensorViz:

```python
import torch
import tenviz

geo = tenviznn.io.read_3dobject("data/mesh/teapot.off")
context = tenviz.Context()

with context.current():
	program = tenviz.DrawProgram(tenviz.DrawMode.Points,
								 vert_shader_file="shaders/simple.vert",
								 frag_shader_file="shaders/simple.frag")
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
```

More examples on the [samples notebook](https://gitlab.com/mipl/3d-reconstruction/tensorviz/-/blob/master/doc/Samples.ipynb).

Current features:

* Directly binding of tensors to shader attributes, uniform or samples;
* Mapping of graphic buffer to Cuda tensor;
* Automatic reloading shaders. Tensorviz recompile shaders while rendering when a file change happens;
* Conversion of texture from and to CUDA;
* Rendering to framebuffers and tensors.

Demos:

* [shadow-mapping demo](https://github.com/otaviog/shadow-mapping)
* [hatching demo](https://github.com/otaviog/hatching-shading)

## Test using Docker

The docker image `otaviog/tensorviz:try` contains all dependencies and its python can be invoked using the [`tv-try`](tv-try)
script.
Example:


```shell
tensorviz$ ./tv-try samples/pointcloud.py
```

## Installing

The project only supports Conda for installing.

### Install dependencies by environment update (conda)

The `environment.yml` contains the dependencies. Installing command line:

```shell
tensorviz$ conda env update -n <env> --file environment.yml
tensorviz$ python setup.py install
```

## Acknowledgments

This work was supported by the [Eldorado Research Institute](https://www.eldorado.org.br/) and the [Machine Intelligence and Perception Research](https://gitlab.com/mipl).
