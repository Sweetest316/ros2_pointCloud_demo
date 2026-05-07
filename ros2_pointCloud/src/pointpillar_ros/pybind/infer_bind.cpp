#include "pointpillar_ros/infer_bind.hpp"
#include <pybind11/embed.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

namespace py = pybind11;

py::object py_infer;

void init_python()
{
    py::initialize_interpreter();

    py::module_ sys = py::module_::import("sys");
    sys.attr("path").attr("append")("src/pointpillar_ros/python");

    py::module_ infer = py::module_::import("infer");
    py_infer = infer.attr("infer");
}

py::dict infer_cpp(std::string path)
{
    py::object result = py_infer(path);
    return result.cast<py::dict>();
}

py::object infer_cpp_vec(const std::vector<float> points)
{
    // size_t N = points.size() / 4;

    // py::array_t<float> np_array(
    //     {N, 4},
    //     {sizeof(float) * 4, sizeof(float)},
    //     points.data()
    // );

    py::object result = py_infer(points);
    return result;
}
