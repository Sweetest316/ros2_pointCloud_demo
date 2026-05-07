cmake_minimum_required(VERSION 3.8)
project(pointpillar_ros)

set(CMAKE_CXX_STANDARD 17)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ROS2
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(visualization_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)

# Python + pybind11
find_package(Python3 REQUIRED COMPONENTS Development)
find_package(pybind11 REQUIRED)

# ✅ 先创建 target（必须在前）
add_executable(pointpillar_node
  src/node.cpp
  pybind/infer_bind.cpp
)

add_executable(pointpillar_sub_node
  src/pubPCNode.cpp
)

# ✅ 再设置 include
target_include_directories(pointpillar_node
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

# ✅ ROS 依赖
ament_target_dependencies(pointpillar_node
  rclcpp
  std_msgs
  visualization_msgs
  geometry_msgs
  sensor_msgs
)

ament_target_dependencies(pointpillar_sub_node 
  rclcpp
  sensor_msgs
)

# ✅ Python embed
target_link_libraries(pointpillar_node
  pybind11::embed
  ${Python3_LIBRARIES}
)

# 安装
install(TARGETS pointpillar_node pointpillar_sub_node
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()