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

# 当前实现

目前已实现：

- [x] ROS2 点云发布
- [x] PointPillars 推理
- [x] pybind11 调用 Python 推理
- [x] OpenPCDet 集成

---

# 后续计划

- [ ] TensorRT 部署
- [ ] CUDA 后处理
- [ ] 多线程推理
- [ ] ros2bag 支持
- [ ] RViz 可视化
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