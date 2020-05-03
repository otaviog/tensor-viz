"""Tests the buffer/tensor handling.
"""
import unittest

import torch

import tenviz


class TestBuffer(unittest.TestCase):
    """Tests the buffer/tensor handling
    """
    @classmethod
    def setUpClass(cls):
        print("Initializing Cuda")
        torch.rand(5, 5).to("cuda:0")  # Initialize torch's cuda system
        print("Done")

    @staticmethod
    def _test_torch_memory_impl(device, dtype):
        context = tenviz.Context()

        tensor = (torch.rand((1024, 3))*255).type(dtype)
        tensor = tensor.to(device)

        with context.current():
            buffer = tenviz.buffer_from_tensor(tensor)
            btensor = buffer.to_tensor().to(device)

        torch.testing.assert_allclose(tensor, btensor)

        indices = torch.tensor(
            [0, 5, 100, 500, 1000, 1021], dtype=torch.int64).to(device)
        slice_tensor = torch.tensor([[1, 1, 1],
                                     [2, 2, 2],
                                     [3, 3, 3],
                                     [4, 4, 4],
                                     [5, 5, 5],
                                     [6, 6, 6]], dtype=dtype).to(device)

        with context.current():
            buffer[indices] = slice_tensor
            btensor = buffer[indices.to("cuda:0")]

        torch.testing.assert_allclose(slice_tensor.cpu(), btensor.cpu())

        # 1D tensor case
        tensor = (torch.rand(1024)*255).type(dtype)

        tensor = tensor.to(device)

        with context.current():
            buffer = tenviz.buffer_from_tensor(tensor)
            btensor = buffer.to_tensor().to(device)

        torch.testing.assert_allclose(tensor, btensor)

        indices = torch.tensor(
            [0, 5, 100, 500, 1000, 1023], dtype=torch.int64).to("cuda:0")
        slice_tensor = torch.tensor([1, 2, 3,
                                     4, 5, 6], dtype=dtype).to(device)

        with context.current():
            buffer[indices.to(device)] = slice_tensor
            btensor = buffer[indices.to("cuda:0")].to(device)

        torch.testing.assert_allclose(slice_tensor.cpu(), btensor.cpu())

    @staticmethod
    def test_torch_memory_cpu():
        """Test torch memory cpu handling.
        """
        TestBuffer._test_torch_memory_impl("cpu:0", torch.float32)
        TestBuffer._test_torch_memory_impl("cpu:0", torch.uint8)

    @staticmethod
    def test_torch_memory_gpu():
        """Test torch memory gpu handling.
        """
        TestBuffer._test_torch_memory_impl("cuda:0", torch.float32)
        TestBuffer._test_torch_memory_impl("cuda:0", torch.uint8)

    @staticmethod
    def test_as_tensor():
        """Test tensor mapping.
        """
        context = tenviz.Context()
        with context.current():
            tensor = (torch.rand((1024, 3))*255).type(torch.float32)
            tensor = tensor.to("cuda:0")

            buff = tenviz.buffer_from_tensor(tensor)
            with buff.as_tensor() as map_tensor:
                map_tensor[:, 1] = 4

            tensor[:, 1] = 4
            torch.testing.assert_allclose(buff.to_tensor(), tensor)


if __name__ == '__main__':
    unittest.main()
