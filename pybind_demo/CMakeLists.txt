cmake_minimum_required(VERSION 3.10)
project(pybind_demo)

set(CMAKE_CXX_STANDARD 14)

find_package(Python3 REQUIRED COMPONENTS Interpreter Development)
find_package(pybind11 REQUIRED)

add_executable(main main.cpp)

target_link_libraries(main
    pybind11::embed
    Python3::Python
)
