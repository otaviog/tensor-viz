#!/usr/bin/env python

from pathlib import Path
import math
from queue import Empty

import torch
import torch.multiprocessing as mp

import tenviz
import tenviz.geometry
import tenviz.io


class SurfelAnimation:
    def __init__(self, verts, normals):
        self.verts = verts
        self.normals = normals
        self.initial_verts = verts.clone()
        self.time = 0

    def __call__(self):
        iverts = self.initial_verts
        self.verts[:, :] = (iverts + self.normals*0.05) * math.sin(self.time)\
            #(torch.sin(iverts[:, 0] + self.time)).view(-1, 1)

        self.time += 0.01


def _animation_proc(verts, normals, queue):
    print(torch.cuda.is_available())
    anim = SurfelAnimation(verts, normals)
    
    while True:
        anim()
        queue.put(True)


def main():
    import argparse
    torch.multiprocessing.set_start_method('spawn')
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "mode", choices=["single", "multi"],
        help="Run mode: single process or multiprocess")
    parser.add_argument(
        "--device", "-d", choices=["cuda:0", "cpu"], default="cuda:0")
    args = parser.parse_args()

    context = tenviz.Context(640, 480)
    mesh = tenviz.io.read_3dobject(
        Path(__file__).parent / "data/mesh/teapot.off")

    mesh.verts = torch.from_numpy(mesh.verts)
    mesh.faces = torch.from_numpy(mesh.faces)
    mesh.normals = tenviz.geometry.compute_normals(mesh.verts, mesh.faces)

    mesh.verts = mesh.verts.to(args.device)
    mesh.normals = mesh.normals.to(args.device)
    surfels = context.add_surfel_cloud()

    with context.current():
        surfels.points.from_tensor(mesh.verts)
        surfels.normals.from_tensor(mesh.normals)

        colors = (torch.abs(torch.rand(mesh.verts.size(0), 3))*255).byte()
        surfels.colors.from_tensor(colors)

        n_verts = mesh.verts.size(0)
        surfels.radii.from_tensor(torch.rand(
            n_verts, 1, dtype=torch.float32).abs()*0.0)

        surfels.mark_visible(torch.arange(0, mesh.verts.shape[0] - 1))

    viewer = context.viewer()
    viewer.reset_view()

    if args.mode == 'multi':
        queue = mp.Queue()
        mesh.verts.share_memory_()

        proc = mp.Process(target=_animation_proc, args=(
            mesh.verts, mesh.normals, queue))
        proc.start()
    else:
        anim = SurfelAnimation(mesh.verts, mesh.normals)

    while True:
        if args.mode == 'multi':
            update = False
            try:
                update = queue.get_nowait()
            except Empty:
                continue
        else:
            anim()
            update = True

        if update:
            context.make_current()
            surfels.points.from_tensor(mesh.verts)
            context.detach_current()

        viewer.draw(0)


if __name__ == '__main__':
    main()
