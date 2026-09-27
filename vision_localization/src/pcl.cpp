

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

// Segmentation du sol

void extractGround(pcl::PointCloud<pcl::PointXYZ>::Ptr cloud) {

    auto ground_cloud = std::make_shared<pcl::PointCloud<pcl::PointXYZ>>();
    auto obstacle_cloud = std::make_shared<pcl::PointCloud<pcl::PointXYZ>>();

    auto coefficients = std::make_shared<pcl::ModelCoeffiocients>();
    auto inliers = std::make_shared<pcl::PointIndices>();

    pcl::SACSegmentation<pcl::PointXYZ> seg;
    seg.setOptimizeCoefficients(true);
    seg.setModelType(pcl::SACMODEL_PLANE);
    seg.setMethodType(pcl::SAC_RANSAC);
    seg.setInputCloud(cloud);
    seg.setDistanceThreshold(0.015);

    seg.segments(*inliers,*coefficients);

    if (inliers->indices.empty()){
        std::cerr << "RANSAC n'a pas pu trouver de plan géometrique" << std:: endl;
        return;
    }

    pcl::ExtractIndices<pcl::PointXYZ> extract;
    extract.setInputCloud(cloud);
    extract.setIndices(inliers);

    //garder le sol
    extract.setNegative(false);
    extract.filter(*ground_cloud);
    
    extract.setNegative(true);
    extract.filter(*obstacle_ground);

}
