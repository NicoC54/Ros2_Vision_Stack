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