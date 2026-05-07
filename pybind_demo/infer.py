import torch
import numpy as np

def infer(data):
    arr = np.array(data, dtype=np.float32)

    point = arr.reshape(-1, 4)
    print("点云shape:", arr.shape)
    
    tensor = torch.from_numpy(point)
    print("tensor shape:", tensor.shape)
    print("device:", tensor.device)
    output = tensor[:, 0].mean()
    return float(output.item())
