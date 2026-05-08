#include "rclcpp/rclcpp.hpp"
#include <pybind11/numpy.h>
#include <iostream>
#include <cmath>
#include <fstream>
#include <mutex>

#include <visualization_msgs/msg/marker_array.hpp>
#include <geometry_msgs/msg/point.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>
#include <sensor_msgs/point_cloud2_iterator.hpp>

#include "pointpillar_ros/infer_bind.hpp"

class NodePP : public rclcpp::Node
{
public:
    NodePP() : Node("pointpillar_node")
    {
        init_python();

        timer_ = this->create_wall_timer(
            std::chrono::milliseconds(1000),
            std::bind(&NodePP::run, this)
        );

        sub_ = this->create_subscription<sensor_msgs::msg::PointCloud2>(
            "/points",
            10,
            std::bind(&NodePP::callback, this, std::placeholders::_1)
        );

        pub_ = this->create_publisher<visualization_msgs::msg::MarkerArray>(
            "bbox", 10
        );

        pc_pub_ = this->create_publisher<sensor_msgs::msg::PointCloud2>(
            "points_processed", 10
        );

        // pc_pub_test = this->create_publisher<sensor_msgs::msg::PointCloud2>(
        //     "points", 10
        // );

    }

    // sensor_msgs::msg::PointCloud2 load_bin_as_pointcloud(std::string path)
    // {
    //     std::ifstream ifs(path, std::ios::binary);
        
    //     ifs.seekg(0, std::ios::end);
    //     size_t file_size = ifs.tellg();
    //     ifs.seekg(0, std::ios::beg);

    //     size_t num_points = file_size / sizeof(float) / 4;

    //     std::vector<float> data(num_points * 4);

    //     ifs.read(reinterpret_cast<char*>(data.data()), file_size);

    //     sensor_msgs::msg::PointCloud2 pc2;
    //     pc2.header.frame_id = "map";
    //     pc2.height = 1;
    //     pc2.width = num_points;
    //     pc2.is_dense = false;
    //     pc2.is_bigendian = false;

    //     sensor_msgs::PointCloud2Modifier modifier(pc2);
    //     modifier.setPointCloud2FieldsByString(1, "xyz");

    //     modifier.resize(num_points);

    //     sensor_msgs::PointCloud2Iterator<float> iter_x(pc2, "x");
    //     sensor_msgs::PointCloud2Iterator<float> iter_y(pc2, "y");
    //     sensor_msgs::PointCloud2Iterator<float> iter_z(pc2, "z");

    //     for (size_t i = 0; i < num_points; ++i, ++iter_x, ++iter_y, ++iter_z)
    //     {
    //         *iter_x = data[i * 4 + 0];
    //         *iter_y = data[i * 4 + 1];
    //         *iter_z = data[i * 4 + 2] + 1.7;
    //     }

    //     return pc2;
    // }


private:
    rclcpp::TimerBase::SharedPtr timer_;

    rclcpp::Subscription<sensor_msgs::msg::PointCloud2>::SharedPtr sub_;

    rclcpp::Publisher<visualization_msgs::msg::MarkerArray>::SharedPtr pub_;
    rclcpp::Publisher<sensor_msgs::msg::PointCloud2>::SharedPtr pc_pub_;

    // rclcpp::Publisher<sensor_msgs::msg::PointCloud2>::SharedPtr pc_pub_test;

    sensor_msgs::msg::PointCloud2::SharedPtr latest_msg_;
    std::mutex mutex_;

    float z_offset = 1.7;
    int idx_ = 0;

    std::string get_path(int idx)
    {
        std::ostringstream oss;
        oss << "/mnt/kitti/KITTI/testing/velodyne/"
            << std::setw(6) << std::setfill('0') << idx
            << ".bin";
        return oss.str();
    }

    void callback(const sensor_msgs::msg::PointCloud2::SharedPtr msg)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        latest_msg_ = msg;
    }
    
    std::vector<float> pc2_to_vec(const sensor_msgs::msg::PointCloud2::SharedPtr msg)
    {
        std::vector<float> pts;

        sensor_msgs::PointCloud2ConstIterator<float> iter_x(*msg, "x");
        sensor_msgs::PointCloud2ConstIterator<float> iter_y(*msg, "y");
        sensor_msgs::PointCloud2ConstIterator<float> iter_z(*msg, "z");

        for (; iter_x != iter_x.end(); ++iter_x, ++iter_y, ++iter_z)
        {
            pts.push_back(*iter_x);
            pts.push_back(*iter_y);
            pts.push_back(*iter_z);
            pts.push_back(0.0); // intensity
        }

        return pts;
    }

    void run()
    {
        // std::string path = get_path(idx_);

        // if (!rcpputils::fs::exists(path)) {
        //     RCLCPP_INFO(this->get_logger(), "FInished all frames");
        //     rclcpp::shutdown();
        //     return;
        // }

        // auto pc_msg = load_bin_as_pointcloud(path);
        // pc_msg.header.stamp = this->get_clock()->now();
        // pc_pub_test->publish(pc_msg);

        // idx_++;

        // RCLCPP_INFO(this->get_logger(),
        //     "Published PointCloud: %d points, frame=%s",
        //     pc_msg.width,
        //     pc_msg.header.frame_id.c_str()
        // );


        sensor_msgs::msg::PointCloud2::SharedPtr msg;

        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!latest_msg_) return;
            msg = latest_msg_;
        }

        auto pts = pc2_to_vec(msg);

        py::object res;
        try {
            res = infer_cpp_vec(pts);
        }
        catch (pybind11::error_already_set &e) {
            std::cerr << "Python exception:\n";
            std::cerr << e.what() << std::endl;
        }

        publish_boxes(res);

        sensor_msgs::PointCloud2Iterator<float> iter_z(*msg, "z");
        for (; iter_z != iter_z.end(); ++iter_z)
        {
            *iter_z += z_offset;
        }
        // msg.header.stamp = this->get_clock()->now();
        pc_pub_->publish(*msg);
    }

    void publish_boxes(py::object res)
    {
        py::array_t<float> boxes = res["pred_boxes"].cast<py::array_t<float>>();
        py::array_t<float> scores = res["pred_scores"].cast<py::array_t<float>>();
        py::array_t<int> labels = res["pred_labels"].cast<py::array_t<int>>();

        auto box_buf = boxes.request();
        auto score_buf = scores.request();
        auto label_buf = labels.request();

        float* box_ptr = (float*)box_buf.ptr;
        float* score_ptr = (float*)score_buf.ptr;
        int* label_ptr = (int*)label_buf.ptr;

        int num = box_buf.shape[0];

        visualization_msgs::msg::MarkerArray marker_array;

        // float z_offset = 1.7;
        int final_num = 0;

        for (int i = 0; i < num; i++) 
        {
            if (score_ptr[i] < 0.5) continue;

            float x = box_ptr[i * 7 + 0];
            float y = box_ptr[i * 7 + 1];
            float z = box_ptr[i * 7 + 2];
            float dx = box_ptr[i * 7 + 3];
            float dy = box_ptr[i * 7 + 4];
            float dz = box_ptr[i * 7 + 5];
            float yaw = box_ptr[i * 7 + 6];

            // int lbl = labels[i];

            visualization_msgs::msg::Marker marker;
            marker.header.frame_id = "map";
            marker.header.stamp = this->get_clock()->now();

            marker.ns = "bbox";
            marker.id = i;
            marker.type = visualization_msgs::msg::Marker::CUBE;
            marker.action = visualization_msgs::msg::Marker::ADD;

            // 位置
            marker.pose.position.x = x;
            marker.pose.position.y = y;
            marker.pose.position.z = z + z_offset;

            // std::cout << "x=" << x << "y=" << y << "z=" << z << " dx=" << dx << " dy=" << dy << " dz=" << dz << std::endl;

            // yaw -> quaternion
            marker.pose.orientation.x = 0.0;
            marker.pose.orientation.y = 0.0;
            marker.pose.orientation.z = sin(yaw / 2);
            marker.pose.orientation.w = cos(yaw / 2);

            // 尺寸
            marker.scale.x = dx;
            marker.scale.y = dy;
            marker.scale.z = dz;

            int label = label_ptr[i];
            if (label == 1) { marker.color.r = 1; }
            if (label == 2) { marker.color.g = 1; }
            if (label == 3) { marker.color.b = 1; }
            marker.color.a = 0.5;

            marker_array.markers.push_back(marker);

            final_num++;
        }

        pub_->publish(marker_array);

        std::cout << "publish: " << final_num << " boxes" << std::endl;
    }

};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<NodePP>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}