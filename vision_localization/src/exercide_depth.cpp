#include <opencv2/opencv.hpp>
#include <iostream>


struct Point3D {float x,y,z;};
struct IntrinsecsMatrix {float cx,cy,fx,fy;};

//extract 3D position of a 2D pixel knowing its row, column and depth.
Point3D Extract3DPosition(const IntrinsecsMatrix& intrinsecMatrix, uint16_t depth_mm, int u, int v){

    if (depth_mm ==0){
        return {0.0f,0.0f,0.0f};
    }

    Point3D point;
    
    point.z = depth/1000.0f; //in meter
    point.y = (u-intrinsecMatrix.cx)*z /intrinsecMatrix.fx; //in meter
    Point.x = (v-intrinsecMatrix.cy)*z /intrinsecMatrix.fy; //in meter

    return point;
}


struct pointCloud {float x,y,z; uint8_t r,g,b;};

pointCloud CreatepointCloud(const cv::Mat& rgb, const cv::Mat& depth, cosnt IntrinsecsMatrix& intrinsecMatrix){

std::vector<pointCloud> cloud;
cloud.reserve(rgb.rows*rgb.cols);

for (int v=0; v<rgb.rows; v++){
    for (int u = 0 ; u < rgb.cols; u++){

        unint16_t depth_mm = depth.at<uint16_t>(v,u);
        if (depth_mm == 0) continue;

        Point3D point = Extract3DPosition(intrinsecMatrix, z, u, v);
        cv::Vec3b color = rgb.at<cv::Vec3b>(v,u);
        
    cloud.push_back({point.x, point.y, point.z, color[2], color[1], color[0]});


    }
}

return cloud;

}


