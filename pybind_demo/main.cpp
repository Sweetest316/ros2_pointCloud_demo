#include <pybind11/embed.h>
#include <pybind11/stl.h>
#include <iostream>
#include <vector>

namespace py = pybind11;

int main() {
    py::scoped_interpreter guard{};  //启动 Python
    try {
        // 加入当前路径（关键）
        py::module sys = py::module::import("sys");
        sys.attr("path").attr("append")("..");

        // 导入 Python 文件
        py::module infer = py::module::import("infer");

        std::vector<float> data = {
            1.0, 2.0, 3.0, 0.5,
            4.0, 5.0, 6.0, 0.6,
            7.0, 8.0, 9.0, 0.7,
            10.0, 11.0, 12.0, 0.8,
            13.0, 14.0, 15.0, 0.9,
            16.0, 17.0, 18.0, 1.0,
            19.0, 20.0, 21.0, 1.0,
        };

        // 调用函数
        py::object result = infer.attr("infer")(data);

        float value = result.cast<float>();

        std::cout << "C++收到结果: " << value << std::endl;

    } catch (std::exception &e) {
        std::cout << "报错: " << e.what() << std::endl;
    }

    return 0;
}
