

#include <pcl_conversions/pcl_conversions.h>
#include <sensor_msgs/msg/point_cloud2.hpp>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>

//Conversion ROS2 PCL

auto cloud = std::make_shared<pcl::PointCloud<pcl::PointXYZ>>();
pcl::fromROSMsg(*ros_msg, *cloud);

//Conversion PCL ROS2

sensor_msgs::msg::PointCloud2 msg;
pcl::toROSmsg(*cloud,*msg);
msg.header.frame_id = "camera_link";

//Ajouter un filtre Voxel



