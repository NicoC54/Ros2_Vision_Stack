#include <opencv2/opencv.hpp>
#include <vector>
#include <opencv2/opencv.hpp>
#include <vector>
#include <memory>
#include <chrono>

// Les includes ROS 2 et TF2 indispensables :
#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/transform_stamped.hpp"
#include "tf2_ros/transform_broadcaster.h"
#include "tf2/LinearMath/Matrix3x3.h"
#include "tf2/LinearMath/Quaternion.h"


class Broadcaster : public rclcpp::Node {


    public:
        Broadcaster() : Node("Broadcaster_node"){

            broadcaster_ = std::make_shared<tf2_ros::TransformBroadcaster>(this);
            timer_ = this->create_wall_timer(std::chrono::milliseconds(500),[this](){this->callbackTimerBroadcaster();});
        }



    void callbackTimerBroadcaster(){
        geometry_msgs::msg::TransformStamped tf;
        cv::Mat rvec,tvec;
        computeArucoPose(rvec, tvec);
        tf.header.stamp = this->get_clock()->now();
        tf.header.frame_id = "camera_link";
        tf.child_frame_id = "aruco_marker";

        tf.transform.translation.x = tvec.at<double>(0);
        tf.transform.translation.y = tvec.at<double>(1);
        tf.transform.translation.z = tvec.at<double>(2);

        cv::Mat cvR; //matrice openCv 3x3
        cv::Rodrigues(rvec,cvR);

        tf2::Matrix3x3 tf2_R(
            cvR.at<double>(0,0), cvR.at<double>(0,1), cvR.at<double>(0,2),
            cvR.at<double>(1,0), cvR.at<double>(1,1), cvR.at<double>(1,2),
            cvR.at<double>(2,0), cvR.at<double>(2,1), cvR.at<double>(2,2)
        );

        tf2::Quaternion q;
        tf2_R.getRotation(q);

        tf.transform.rotation.x = q.x();
        tf.transform.rotation.y = q.y();
        tf.transform.rotation.z = q.z();
        tf.transform.rotation.w = q.w();

        broadcaster_->sendTransform(tf);

    }

void computeArucoPose(cv::Mat& rvec, cv::Mat& tvec) {
    float marker_size = 0.10f;

    std::vector<cv::Point3f> objectPoints = {
            cv::Point3f(-marker_size/2,  marker_size/2, 0), // Haut gauche
            cv::Point3f( marker_size/2,  marker_size/2, 0), // Haut droite
            cv::Point3f( marker_size/2, -marker_size/2, 0), // Bas droite
            cv::Point3f(-marker_size/2, -marker_size/2, 0)  // Bas gauche
        };



    // 2. Points 2D détectés sur l'image (obtenus via cv::aruco::detectMarkers)
        std::vector<cv::Point2f> imagePoints = {
            cv::Point2f(320, 240),
            cv::Point2f(420, 245),
            cv::Point2f(415, 345),
            cv::Point2f(315, 340)
        };


    // 3. Matrice intrinsèque (Obtenue par calibration préalable)
        cv::Mat cameraMatrix = (cv::Mat_<double>(3,3) << 
            600.0, 0.0, 320.0,
            0.0, 600.0, 240.0,
            0.0, 0.0, 1.0);   


        cv::Mat distCoeffs = cv::Mat::zeros(4, 1, CV_64F);

        cv::solvePnP(objectPoints, imagePoints, cameraMatrix, distCoeffs, rvec, tvec);

        std::cout << "Translation (X, Y, Z en mètres) : " << tvec << std::endl;
        std::cout << "Rotation (Vecteur de Rodrigues) : " << rvec << std::endl;
    }

        std::shared_ptr<tf2_ros::TransformBroadcaster> broadcaster_;
        std::shared_ptr<rclcpp::TimerBase> timer_;


};

int main(int argc, char* argv[]){

    rclcpp::init(argc,argv);

    rclcpp::spin(std::make_shared<Broadcaster>());
    rclcpp::shutdown();

}