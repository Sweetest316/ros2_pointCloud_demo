#include "rclcpp/rclcpp.hpp"
#include <iostream>
#include <fstream>

// #include <geometry_msgs/msg/point.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>
#include <sensor_msgs/point_cloud2_iterator.hpp>
#include <rcpputils/filesystem_helper.hpp>

class NodePP : public rclcpp::Node
{
public:
    NodePP() : Node("pointpillar_sub_node")
    {
        timer_ = this->create_wall_timer(
            std::chrono::milliseconds(1000),
            std::bind(&NodePP::run, this)
        );

        pc_pub_ = this->create_publisher<sensor_msgs::msg::PointCloud2>(
            "points", 10
        );

    }


private:
    rclcpp::TimerBase::SharedPtr timer_;
    rclcpp::Publisher<sensor_msgs::msg::PointCloud2>::SharedPtr pc_pub_;

    int idx_ = 0;

    sensor_msgs::msg::PointCloud2 load_bin_as_pointcloud(std::string path)
    {
        std::ifstream ifs(path, std::ios::binary);
        
        ifs.seekg(0, std::ios::end);
        size_t file_size = ifs.tellg();
        ifs.seekg(0, std::ios::beg);

        size_t num_points = file_size / sizeof(float) / 4;

        std::vector<float> data(num_points * 4);

        ifs.read(reinterpret_cast<char*>(data.data()), file_size);

        sensor_msgs::msg::PointCloud2 pc2;
        pc2.header.frame_id = "map";
        pc2.height = 1;
        pc2.width = num_points;
        pc2.is_dense = false;
        pc2.is_bigendian = false;

        sensor_msgs::PointCloud2Modifier modifier(pc2);
        // modifier.setPointCloud2FieldsByString(1, "xyz");
        modifier.setPointCloud2Fields(
            4, 
            "x", 1, sensor_msgs::msg::PointField::FLOAT32,
            "y", 1, sensor_msgs::msg::PointField::FLOAT32,
            "z", 1, sensor_msgs::msg::PointField::FLOAT32,
            "intensity", 1, sensor_msgs::msg::PointField::FLOAT32
        );

        modifier.resize(num_points);

        sensor_msgs::PointCloud2Iterator<float> iter_x(pc2, "x");
        sensor_msgs::PointCloud2Iterator<float> iter_y(pc2, "y");
        sensor_msgs::PointCloud2Iterator<float> iter_z(pc2, "z");
        sensor_msgs::PointCloud2Iterator<float> iter_i(pc2, "intensity");

        for (size_t i = 0; i < num_points; ++i, ++iter_x, ++iter_y, ++iter_z, ++iter_i)
        {
            *iter_x = data[i * 4 + 0];
            *iter_y = data[i * 4 + 1];
            *iter_z = data[i * 4 + 2];
            *iter_i = data[i * 4 + 3];
        }

        return pc2;
    }

    std::string get_path(int idx)
    {
        std::ostringstream oss;
        oss << "/mnt/kitti/zjb_public_data/KITTI/testing/velodyne/"
            << std::setw(6) << std::setfill('0') << idx
            << ".bin";
        return oss.str();
    }

    void run()
    {
        std::string path = get_path(idx_);

        if (!rcpputils::fs::exists(path)) {
            RCLCPP_INFO(this->get_logger(), "FInished all frames");
            rclcpp::shutdown();
            return;
        }

        auto pc_msg = load_bin_as_pointcloud(path);
        pc_msg.header.stamp = this->get_clock()->now();
        pc_pub_->publish(pc_msg);

        idx_++;

        RCLCPP_INFO(this->get_logger(),
            "Published PointCloud: %d points, frame=%s",
            pc_msg.width,
            pc_msg.header.frame_id.c_str()
        );
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