# ROS2 PointCloud Inference Demo

基于 ROS2 的点云发布与 PointPillars 推理项目。

当前项目通过 `pybind11` 调用 Python 侧 OpenPCDet 模型进行推理，实现：

- ROS2 点云发布
- ROS2 点云订阅
- PointPillars 点云目标检测
- pybind11 C++ / Python 混合推理
- OpenPCDet 集成

---

# 项目结构

```bash
projects/
├── OpenPCDet/             # OpenPCDet 源码
├── ros2_pointCloud/       # ROS2 工程
├── pybind_demo/           # pybind11 示例
└── .gitignore
```

---

# 环境要求

## 系统

- Ubuntu 22.04
- ROS2 Humble
- Python 3.10
- CUDA 11.x

## Python

安装 OpenPCDet 依赖：

```bash
cd OpenPCDet
pip install -r requirements.txt
python setup.py develop
```

---

# 编译 

```bash
cd ros2_pointCloud

colcon build --symlink-install
```

source：

```bash
source install/setup.bash
```

---

# 运行方式

## 1. 启动点云发布

```bash
ros2 run pointpillar_ros pointpillar_sub_node
```

## 2. 启动推理节点

```bash
ros2 run pointpillar_ros pointpillar_node
```

---


# ros2bag

项目支持使用 `ros2bag` 对 ROS2 Topic 进行录制与回放。

推荐将 bag 数据统一存放到独立数据目录中：

```bash
projects/
├── data/
│   ├── bags/
│   │   ├── kitti-pc-demo/
│   │   └── ...
```

---

## 录制 PointCloud2 Topic

启动点云发布节点：

```bash
ros2 run pointpillar_ros pointpillar_sub_node
```

录制 `/points` topic：

```bash
ros2 bag record \
    -o ~/projects/data/bags/kitti-pc-demo \
    /points
```

录制完成后按：

```bash
Ctrl + C
```

生成：

```bash
kitti-pc-demo/
├── metadata.yaml
├── *.db3
```

---

## 播放 ros2bag

播放 bag：

```bash
ros2 bag play ~/projects/data/bags/kitti-pc-demo
```

循环播放：

```bash
ros2 bag play \
    ~/projects/data/bags/kitti-pc-demo \
    --loop
```

调整播放速度：

```bash
ros2 bag play \
    ~/projects/data/bags/kitti-pc-demo \
    --rate 0.5
```

---

# 当前实现

目前已实现：

- [x] ROS2 点云发布
- [x] PointPillars 推理
- [x] pybind11 调用 Python 推理
- [x] OpenPCDet 集成
- [x] ros2bag 支持
- [x] RViz 可视化

---

# 后续计划

- [ ] TensorRT 部署
- [ ] CUDA 后处理
- [ ] 多线程推理
- [ ] CenterPoint 支持
- [ ] BEVFusion 支持

---

# 注意事项

## 推理频率

当前 ROS2 点云发布频率与推理频率建议保持一致。

否则可能出现：

- 数据竞争
- Tensor shape 异常
- 空点云访问
- 多线程同步问题

---

# 参考项目

- [OpenPCDet](https://github.com/open-mmlab/OpenPCDet)
- ROS2
- pybind11

---

# License

MIT License