#pragma once

#include <string>
#include <pybind11/pybind11.h>

namespace py = pybind11;

void init_python();

py::dict infer_cpp(std::string path);

py::object infer_cpp_vec(std::vector<float> pts);