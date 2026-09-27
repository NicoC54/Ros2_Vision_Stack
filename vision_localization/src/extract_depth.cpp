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