#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
#include <vector>
#include <memory>
#include <chrono>

// Includes ROS 2 et TF2
#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/transform_stamped.hpp"
#include "tf2_ros/transform_broadcaster.h"
#include "tf2/LinearMath/Matrix3x3.h"
#include "tf2/LinearMath/Quaternion.h"

class ExtractArucoBroadcaster : public rclcpp::Node {
public:
    ExtractArucoBroadcaster() : Node("ExtractArucoBroadcaster") {

        broadcaster_ = std::make_shared<tf2_ros::TransformBroadcaster>(this);
        timer_ = this->create_wall_timer(std::chrono::milliseconds(500),[this](){this->callbackTimer();});

        // 1. Initialisation de la vision
        CalibrateCamera();
        ExtractArucoCorners();
        PnP();
    }

private:
    cv::Mat MatrixIntra;
    cv::Mat MatrixDistortion;
    std::vector<std::vector<cv::Point2f>> corners;
    std::vector<cv::Mat> tvecs;
    std::vector<cv::Mat> rvecs;
    std::shared_ptr<tf2_ros::TransformBroadcaster> broadcaster_;
    std::shared_ptr<rclcpp::TimerBase> timer_;
    
    cv::String image_path = "/home/nico/vision_ws/src/ros2_vision_stack/vision_localization/aruco.jpg";

    void CalibrateCamera() {
        std::vector<std::vector<cv::Point3f>> ObjPoints;
        std::vector<std::vector<cv::Point2f>> ImgPoints;
        double sizeSquare = 0.01;
        cv::Size Board(10, 10);

        std::vector<cv::Point3f> Board3D;
        for (int i = 0; i < Board.height; i++) {
            for (int j = 0; j < Board.width; j++) {
                Board3D.push_back(cv::Point3f(j * sizeSquare, i * sizeSquare, 0.0f));
            }
        }

        cv::Size ImageSize;
        std::vector<cv::String> fileNames; 
        cv::glob("dossier_image/*.jpg", fileNames);
        
        for (const auto& file : fileNames) {
            cv::Mat Image = cv::imread(file);
            if(Image.empty()) continue;
            
            cv::Mat GrayImage;
            cv::cvtColor(Image, GrayImage, cv::COLOR_BGR2GRAY);
            ImageSize = GrayImage.size();

            std::vector<cv::Point2f> temp_corners;
            bool full = cv::findChessboardCorners(GrayImage, Board, temp_corners);

            if (full) {
                cv::cornerSubPix(GrayImage, temp_corners, cv::Size(11,11), cv::Size(-1,-1), 
                                 cv::TermCriteria(cv::TermCriteria::EPS | cv::TermCriteria::MAX_ITER, 30, 0.1));
                ObjPoints.push_back(Board3D);
                ImgPoints.push_back(temp_corners);
            }
        }

        std::vector<cv::Mat> calib_rvecs, calib_tvecs;
        cv::calibrateCamera(ObjPoints, ImgPoints, ImageSize, MatrixIntra, MatrixDistortion, calib_rvecs, calib_tvecs);
    }

    void ExtractArucoCorners() {
        cv::Mat Aruco_BGR_Image = cv::imread(image_path);
        if(Aruco_BGR_Image.empty()) return;

        cv::Mat Aruco_Gray_Image;
        cv::cvtColor(Aruco_BGR_Image, Aruco_Gray_Image, cv::COLOR_BGR2GRAY);

        cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_50);
        std::vector<int> ids;
        
        cv::aruco::detectMarkers(Aruco_Gray_Image, dictionary, corners, ids);
    }

    void PnP() {
        if(corners.empty()) {
            return;
        } 

        double marker_size = 0.1;
        std::vector<cv::Point3f> Aruco_dim = {
            cv::Point3f(-marker_size/2,  marker_size/2, 0.0f),
            cv::Point3f( marker_size/2,  marker_size/2, 0.0f),
            cv::Point3f( marker_size/2, -marker_size/2, 0.0f),
            cv::Point3f(-marker_size/2, -marker_size/2, 0.0f)
        };

        tvecs.clear();
        rvecs.clear();

        for (size_t i = 0; i < corners.size(); i++) {
            cv::Mat current_tvec, current_rvec;
            cv::solvePnP(Aruco_dim, corners[i], MatrixIntra, MatrixDistortion, current_rvec, current_tvec);
            tvecs.push_back(current_tvec);
            rvecs.push_back(current_rvec);
        }
    }

    void callbackTimer() {

        if (rvecs.empty() || tvecs.empty()){
            return;  
        } 
        
        for (size_t i = 0; i < rvecs.size(); i++){

            geometry_msgs::msg::TransformStamped tf;

            tf.header.stamp = this->get_clock()->now();
            tf.header.frame_id = "camera_link";
            tf.child_frame_id = "aruco_pose_"+ std::to_string(i);

            tf.transform.translation.x = tvecs[i].at<double>(0);
            tf.transform.translation.y = tvecs[i].at<double>(1);
            tf.transform.translation.z = tvecs[i].at<double>(2);

            cv::Mat cvR;
            cv::Rodrigues(rvecs[i],cvR);

            tf2::Matrix3x3 tf2Mat(
                cvR.at<double>(0,0), cvR.at<double>(0,1), cvR.at<double>(0,2),
                cvR.at<double>(1,0), cvR.at<double>(1,1), cvR.at<double>(1,2),
                cvR.at<double>(2,0), cvR.at<double>(2,1), cvR.at<double>(2,2)
            );

            tf2::Quaternion q;
            tf2Mat.getRotation(q);

            tf.transform.rotation.x = q.x();
            tf.transform.rotation.y = q.y();
            tf.transform.rotation.z = q.z();
            tf.transform.rotation.w = q.w();

            broadcaster_->sendTransform(tf);
        }
       

    }
};

int main(int argc, char* argv[]) {
    rclcpp::init(argc, argv);
    
    auto node = std::make_shared<ExtractArucoBroadcaster>();
    
    rclcpp::spin(node);
    rclcpp::shutdown();
    
    return 0;
}