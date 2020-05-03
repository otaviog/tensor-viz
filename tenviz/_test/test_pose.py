"""Test the pose classes.
"""

import unittest

import torch

from tenviz.pose import Pose, PoseDict


class TestPose(unittest.TestCase):
    """Test the pose class.
    """

    @staticmethod
    def test_from_to_matrix():
        """Test Matrix -> Pose.
        """
        pose_matrix = torch.tensor([
            [0.2918482, 0.4457977, -0.8462204, 5],
            [-0.4647315, 0.8393747, 0.2819127, 4],
            [0.8359721, 0.3109895, 0.4521463, 9],
            [0, 0, 0, 1]
        ], dtype=torch.double)

        pose = Pose.from_matrix(pose_matrix)

        pose_matrix1 = pose.to_matrix()
        torch.testing.assert_allclose(pose_matrix, pose_matrix1)

    @staticmethod
    def test_inverse():
        """Test pose inverse.
        """
        pose_matrix = torch.tensor([
            [0.2918482, 0.4457977, -0.8462204, 5],
            [-0.4647315, 0.8393747, 0.2819127, 4],
            [0.8359721, 0.3109895, 0.4521463, 9],
            [0, 0, 0, 1]
        ], dtype=torch.double)

        pose = Pose.from_matrix(pose_matrix)

        inv_pose = pose.inverse()
        torch.testing.assert_allclose(
            torch.eye(4, dtype=torch.double),
            inv_pose.to_matrix() @ pose_matrix,
            rtol=1e-02, atol=0.2)

    @staticmethod
    def test_matmul():
        """Is matmul (@ operator) working.
        """
        pose0_mtx = torch.tensor([
            [0.2918482, 0.4457977, -0.8462204, 5],
            [-0.4647315, 0.8393747, 0.2819127, 4],
            [0.8359721, 0.3109895, 0.4521463, 9],
            [0, 0, 0, 1]
        ], dtype=torch.double)

        pose1_mtx = torch.tensor([
            [0.4949787, 0.7560792, -0.4281827, 8],
            [0.6390744, 0.0170947, 0.7689549, 3],
            [0.5887104, -0.6542569, -0.4747294, 11],
            [0, 0, 0, 1]], dtype=torch.double)

        pose0 = Pose.from_matrix(pose0_mtx)
        pose1 = Pose.from_matrix(pose1_mtx)

        pose2 = pose0 @ pose1

        torch.testing.assert_allclose(
            pose0_mtx @ pose1_mtx, pose2.to_matrix())

    @staticmethod
    def test_interpolate():
        """Test the pose interpolation.
        """
        pose0 = Pose.from_matrix(torch.tensor([
            [0.2918482, 0.4457977, -0.8462204, 5],
            [-0.4647315, 0.8393747, 0.2819127, 4],
            [0.8359721, 0.3109895, 0.4521463, 9],
            [0, 0, 0, 1]
        ], dtype=torch.double))

        pose1 = Pose.from_matrix(torch.tensor([
            [0.4949787, 0.7560792, -0.4281827, 8],
            [0.6390744, 0.0170947, 0.7689549, 3],
            [0.5887104, -0.6542569, -0.4747294, 11],
            [0, 0, 0, 1]], dtype=torch.double))

        pose01 = pose0.interpolate(pose1, 0.5)

        torch.testing.assert_allclose(
            pose01.to_matrix(),
            torch.tensor([[0.3088, 0.6858, -0.6591, 6.5430],
                          [0.1535, 0.6479, 0.7461, 3.3397],
                          [0.9387, -0.3315, 0.0948, 9.8553],
                          [0.0000, 0.0000, 0.0000, 1.0000]],
                         dtype=torch.float64),
            rtol=1e-04, atol=0.4)


class TestPoseDict(unittest.TestCase):
    """Test the pose dictionary class.
    """

    def test_add(self):
        """Testing add a new pose to a pose dictionary.
        """
        pose_dict = PoseDict()

        pose_dict[1.5] = Pose.from_matrix(torch.eye(4))

        pose2 = Pose.from_matrix(torch.tensor([
            [0, 0, 1, 5],
            [1, 0, 0, 4.0],
            [0, 1, 0, 3],
            [0, 0, 0, 1]], dtype=torch.double))
        pose_dict[2.9] = pose2

        pose_dict.add(3.0, pose2)

        self.assertEqual(3, len(pose_dict))


if __name__ == '__main__':
    unittest.main()
