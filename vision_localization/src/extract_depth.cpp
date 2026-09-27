//Extract depth from a pixel in a depth image

#include <opencv2/opencv.hpp>
#include <iostream>

void extractDepth(const cv::Mat& depth_image, int u, int v){
    if (depth_image.type()!= CV_16U){
        std::cerr << "Format incorrect. CV_16U attendu." << std::endl;
        return;   
}

uint16_t depth_mm = depth_image.at<unint_16t>(v,u);

if (depth_mm > 0 ){
    std::cout << "Distance :" <<depth_m << "mm "
}
}

// Get a ros2 depth message image with cv bridge

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <cv_bridge/cv_bridge.h>

void imageCallback(const sensor_msgs::msg::Image::SharedPtr msg){
    try[

        cv_bridge::CvImagePtr cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encoding::TYPE_16UC1);
        cv::Mat Depth_image = cv_ptr -> image;
    ] catch (cv::bridge::Exception& e) {
        RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "Erreur cv_bridge: %s", e.what());
    }
}


//Deprojection : from a pixel of a 2D depth image get the distance of the object from the camera in 3D Space 

Struct Point3D {float x,y,z;};
Struct CameraIntrinsics {float fx,fy,cx,cy;};

Point3D deprojecPixelto3D(int u, int v, uint16_t depth_mm, const CameraIntrinsics& intrinsics){
    Point3D point;
    point.z = depth_mm/1000.0f;

    if (point.z == 0.0){
        return (0.0,0.0,0.0);
    }

    point.x = (u - intrinsics.cx) * point.z / intrinsics.fx;
    point.y = (v - intrinsics.cy) * point.z / intrinsics.fy;
    return point;
}

//Generate a PointCloud

#include <vector>

struct PointXYZRGB {float x,y,z; uint8_t r,g,b;};

std::vector<PointXYZRGB> generatePointCloud(const cv::Mat& rgb, const cv::Mat& depth, const CameraIntrinsics& intr){
    std::vector<PointXYZRGB> cloud;
    cloud.reserve(rgb.rows * rgb.cols)

    for (int v = 0; v < rgb.rows; ++v){
        for (int u = 0; u < rgb.cols; ++u){
            uint16_t d = depth.at<uint16_t>(v,u);
            if (d==0) continue;

            Point3D p3d = deprojectPixelTo3D(u ,v ,d , intr); // appel de la fonction du dessus

            cv::Vec3b Color = rgb.at<cv::Vec3b> (v,u);

            cv::Vec3b color = rgb.at<cv::Vec3b>(v,u);

            cloud.push_back({p3d.x, p3d.y, p3d.z, color[2], color[1], color[0]})
        }
    }
}