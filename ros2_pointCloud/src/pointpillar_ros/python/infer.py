# infer.py

import sys
sys.path.append("/home/zxc20/projects/OpenPCDet")

from tools.infer_simple import PointPillarInfer

model = PointPillarInfer(
    "/home/zxc20/projects/OpenPCDet/tools/cfgs/kitti_models/pointpillar.yaml",
    "/home/zxc20/projects/OpenPCDet/checkpoints/pointpillar_7728.pth"
)

def infer(bin_pts):
    # if not isinstance(bin_pts, str):
    #     if bin_pts.shape[0] == 0:
    #         print("Empty point cloud!")
    #         return None
    # print("points shape:", bin_pts)

    res = model.infer(bin_pts)

    out = {}
    for k, v in res.items():
        if hasattr(v, "device"):
            out[k] = v.detach().cpu().numpy()
        else:
            out[k] = v.numpy()

    return out

if __name__ == '__main__':
    res = infer("/mnt/kitti/KITTI/testing/velodyne/000000.bin")
    print(res)