#include <opencv2/opencv.hpp>
#include <iostream>

int main(){

    cv::Mat image_bgr = cv::imread("/home/nico/vision_ws/src/ros2_vision_stack/vision_localization/balle_rouge.jpg");

    if (image_bgr.empty()){
        std::cerr << "Erreur:Image introuvable !" << std::endl;
        return -1;
    }
    else{
        std::cout << "Image trouvée !" << std::endl;
        //cv::imshow("Test_balle",image_bgr);
    }

    cv::Mat image_hsv;

    cv::cvtColor(image_bgr, image_hsv, cv::COLOR_BGR2HSV);

    cv::Scalar limite_basse1(0,120,70);
    cv::Scalar limite_haute1(10,255,255);

    cv::Scalar limite_basse2(160,120,70);
    cv::Scalar limite_haute2(179,250,255);

    cv::Mat masque1, masque2, masque_final;

    cv::inRange(image_hsv, limite_basse1, limite_haute1, masque1);
    cv::inRange(image_hsv, limite_basse2, limite_haute2, masque2);

    cv::bitwise_or(masque1,masque2,masque_final);

    cv::namedWindow("image originale bgr", cv::WINDOW_NORMAL);
    cv::imshow("image originale bgr", image_bgr);

    cv::namedWindow("image hsv", cv::WINDOW_NORMAL);
    cv::imshow("image hsv", image_hsv);
    
    cv::namedWindow("masque rouge final", cv::WINDOW_NORMAL);
    cv::imshow("masque rouge final", masque_final);

    cv::waitKey(0);

    return 0;


}