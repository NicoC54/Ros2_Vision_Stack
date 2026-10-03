# ROS 2 Vision Stack : From Pixels to Point Clouds

This repository serves as both a **comprehensive knowledge base** and a **practical C++ workspace** for Computer Vision in robotics. It bridges the gap between raw OpenCV image processing and advanced 3D spatial localization within the ROS 2 ecosystem.

## Purpose
As a Robotics Engineer, I built this repository to systematically document and implement the complete computer vision pipeline: starting from basic memory management of images to advanced 3D Point Cloud manipulation and Perspective-n-Point (PnP) algorithms.

## Workspace Architecture

This workspace is divided into two main ROS 2 packages:

### 1. `vision_core` (Foundations)
Focuses on the fundamentals of image handling between ROS 2 and OpenCV.
* **`cv_bridge` Integration:** Converting ROS 2 sensor messages to OpenCV matrices.
* **Memory Management:** Deep dive into `cv::Mat` architecture and efficient image processing without memory leaks.

### 2. `vision_localization` (Spatial Intelligence)
Focuses on extracting actionable 3D data and robotic pose estimation from 2D images and depth sensors.
* **Classical Vision:** Morphological operations, color space segmentations, and Canny edge detection.
* **Camera Calibration:** Intrinsic and extrinsic parameter extraction.
* **Fiducial Markers & Pose Estimation:** Detecting ArUco markers and computing 6D poses using the PnP (Perspective-n-Point) algorithm.
* **3D Vision:** Integrating Depth cameras, Point Cloud Library (PCL) processing, and conceptual bridges to Visual SLAM.

## Documentation & Theory
I strongly believe in understanding the math behind the code. Each module is accompanied by detailed Markdown documentation (in the `/documentation` folders) covering the theoretical concepts, limitations, and mathematical foundations of the algorithms used before they are implemented in C++.

## Technologies Used
* **C++17 / ROS 2**
* **OpenCV**
* **PCL (Point Cloud Library)**
* **CMake**
