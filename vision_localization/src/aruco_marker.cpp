#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
#include <iostream>
#include <vector>

int main(){

    std::cout <<"aruco_marker_executable" <<std::endl;

    cv::Mat aruco_image = cv::imread ("/home/nico/vision_ws/src/ros2_vision_stack/vision_localization/aruco.jpg");
    cv::Mat aruco_image_grey;
    cv::cvtColor(aruco_image, aruco_image_grey, cv::COLOR_BGR2GRAY);
    
    
    std::vector<std::vector<cv::Point2f>> Aruco_corners; //vecteur contenant un vecteur des 4 poses (x,y) de coins de chaque aruco pour chaque aruco
    std::vector<int> ids;

    cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_50);

    cv::aruco::detectMarkers(aruco_image_grey, dictionary, Aruco_corners, ids);

    if (ids.empty()){
        std::cout << "pas de tags aruco détecté" <<std::endl;
        return -1;
    }
    //pas besoin de else

    std::cout << "Tags: détectés: " << ids.size() <<std::endl; 

    for (size_t i=0; i < ids.size(); i++){
        std::cout << "tag numéro: " << ids[i] << " détecté" <<std::endl;
        std::cout << "coin_haut_gauche x: " <<  Aruco_corners[i][0].x << ", y: " << Aruco_corners[i][0].y << std::endl;
        std::cout << "coin_haut_droite x: " <<  Aruco_corners[i][1].x << ", y: " << Aruco_corners[i][1].y << std::endl;
        std::cout << "coin_bas_droite x: " <<  Aruco_corners[i][2].x << ", y: " << Aruco_corners[i][2].y << std::endl;
        std::cout << "coin_bas_gauche x: " <<  Aruco_corners[i][3].x << ", y: " << Aruco_corners[i][3].y << std::endl;
    }

    cv::aruco::drawDetectedMarkers(aruco_image, Aruco_corners, ids);
    cv::imshow("image_aruco",aruco_image);
    cv::waitKey(0);

    return 0;
}