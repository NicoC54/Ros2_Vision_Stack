#include <opencv2/opencv.hpp>
#include <vector>
#include <iostream>

void featureMatchingAndRANSAC(const cv::Mat& img1, const cv::Mat img2){

    cv::Ptr<cv::ORB> orb = cv::ORB::create();

    std::vector<cv::KeyPoint> keypoints1, keypoints2;
    cv::Mat descriptors1, descriptors2;

    orb -> detectAndCompute(img1, cv::noArray(), keypoints1, descriptors1);
    orb -> detectAndCompute(img2, cv::noArray(), keypoints2, descriptors2);

    cv::BFMatcher matcher(cv::NORM_HAMMING,true);
    std::vector<cv::Dmatch> matches;
    matcher.match(descriptors1, descriptors2, matches)

    std::vector<cv::Point2f> points1, points2;
    for (size_t i = 0; i < matches.size(); i++) {
        points1.push_back(keypoints1[matches[i].queryIdx].pt);
        points2.push_back(keypoints2[matches[i].trainIdx].pt);
    }
    std::vector<uchar> inliers_mask;
    cv::Mat homography = cv::findHomography(points1, points2, cv::RANSAC, 3.0, inliers_mask)

     // Comptage des bons matchs validés
    int inliers_count = 0;
    for (size_t i = 0; i < inliers_mask.size(); i++) {
        if (inliers_mask[i]) inliers_count++;
    }

    std::cout << "Matchs bruts (avec erreurs) : " << matches.size() << std::endl;
    std::cout << "Bons matchs (Inliers RANSAC) : " << inliers_count << std::endl;

}