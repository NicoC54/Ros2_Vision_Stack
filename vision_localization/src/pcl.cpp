

#include <pcl_conversions/pcl_conversions.h>
#include <sensor_msgs/msg/point_cloud2.hpp>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>

// Conversion ROS2 PCL

auto cloud = std::make_shared<pcl::PointCloud<pcl::PointXYZ>>();
pcl::fromROSMsg(*ros_msg, *cloud);

// Conversion PCL ROS2

sensor_msgs::msg::PointCloud2 msg;
pcl::toROSmsg(*cloud,*msg);
msg.header.frame_id = "camera_link";

// Filtre Voxel

pcl::PointCloud<pcl::PointXYZ>::Ptr ApplyFilterVoxel(pcl::PointCloud<pcl::pointXYZ>::Ptr cloud){

    std::shared_ptr<pcl::PointCloud<pcl::PointXYZ>> filtered_cloud = std::make_shared<pcl::PointCloud<pcl::PointXYZ>>();

    pcl::VoxelGrid<PointXYZ> voxel_filter;

    voxel_filter.setInputCloud(cloud);
    voxel_filter.setLeafSize(0.02,0.02,0.02);
    voxel_filter.filter(*filtered_cloud);

    return filtered_cloud;
}
