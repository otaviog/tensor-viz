about:
	@echo "TensorViz unit testing"

all:
	python -m unittest discover

tenviz.io:
	python3 -m unittest tenviz.io._test

tenviz.geometry:
	python3 -m unittest tenviz._test.test_geometry

tenviz.projection:
	python3 -m unittest tenviz._test.test_projection

tenviz.buffer:
	python3 -m unittest tenviz._test.test_buffer

tenviz.buffer.gpu:
	python3 -m unittest tenviz._test.test_buffer.TestBuffer.test_torch_memory_gpu

tenviz.buffer.cpu:
	python3 -m unittest tenviz._test.test_buffer.TestBuffer.test_torch_memory_cpu

tenviz.buffer.as_tensor:
	python3 -m unittest tenviz._test.test_buffer.TestBuffer.test_as_tensor

tenviz.draw_program:
	python3 -m unittest tenviz._test.test_draw_program

tenviz.context:
	python3 -m unittest tenviz._test.test_context

tenviz.program:
	python3 -m unittest tenviz._test.test_program

tenviz.texture:
	python3 -m unittest tenviz._test.test_texture

tenviz.texture.texture2d:
	python3 -m unittest tenviz._test.test_texture.TestTexture.test_texture_2d

tenviz.texture.texture3d:
	python3 -m unittest tenviz._test.test_texture.TestTexture.test_texture_3d

tenviz.pose.PoseDict:
	python3 -m unittest tenviz._test.test_pose.TestPoseDict

tenviz.pose.Pose:
	python3 -m unittest tenviz._test.test_pose.TestPose


