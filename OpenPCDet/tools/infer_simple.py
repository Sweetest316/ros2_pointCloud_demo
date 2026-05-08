import os
import torch
import numpy as np

from pcdet.config import cfg, cfg_from_yaml_file
from pcdet.datasets import DatasetTemplate
from pcdet.models import build_network, load_data_to_gpu
from pcdet.utils import common_utils


class DemoDataset(DatasetTemplate):
    def __init__(self, dataset_cfg, class_names, training=False):
        super().__init__(dataset_cfg=dataset_cfg, class_names=class_names, training=training)
        self.file_list = []

    def set_file(self, file):
        self.file_list = [file]

    def set_pts(self, pts):
        self.file_list = [pts]

    def __len__(self):
        return len(self.file_list)

    def __getitem__(self, index):
        file = self.file_list[index]
        if isinstance(file, str):
            points = np.fromfile(file, dtype=np.float32).reshape(-1, 4)
        else:
            points = np.array(file, dtype=np.float32).reshape(-1, 4) 

        # print(points.shape)
        # print(points[:5])

        # print(points[:, 0].min(), points[:, 0].max())
        # print(points[:, 1].min(), points[:, 1].max())
        # print(points[:, 2].min(), points[:, 2].max())

        input_dict = {
            'points': points,
            'frame_id': 0,
        }

        data_dict = self.prepare_data(data_dict=input_dict)

        return data_dict

class PointPillarInfer:
    def __init__(self, cfg_file, ckpt):
        logger = common_utils.create_logger()
        logger.info('-----------------Quick Demo of OpenPCDet-------------------------')

        root = "/home/zxc20/projects/OpenPCDet/tools"
        os.chdir(root)

        cfg_from_yaml_file(cfg_file, cfg)

        self.dataset = DemoDataset(cfg.DATA_CONFIG, cfg.CLASS_NAMES, training=False)
        self.model = build_network(cfg.MODEL, num_class=len(cfg.CLASS_NAMES), dataset=self.dataset)
        self.model.load_params_from_file(filename=ckpt, logger=logger, to_cpu=True)
        self.model.cuda()
        self.model.eval()

    def infer(self, bin_path):
        if isinstance(bin_path, str):
            self.dataset.set_file(bin_path)
        else:
            self.dataset.set_pts(bin_path)
        data_dict = self.dataset[0]
        data_dict = self.dataset.collate_batch([data_dict])
        # print(data_dict['voxel_coords'].shape)
        # print(data_dict['voxel_coords'][:5])
        load_data_to_gpu(data_dict)
        with torch.no_grad():
            preds, _ = self.model(data_dict)
        return preds[0]

