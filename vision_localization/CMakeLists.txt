cmake_minimum_required(VERSION 3.8)
project(vision_localization)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_ros REQUIRED)

find_package(OpenCV REQUIRED)

add_executable(color_and_segmentation src/color_spaces_segmentation.cpp)
add_executable(morphological_transformations src/morphological_transformations.cpp)

ament_target_dependencies(
color_and_segmentation
rclcpp
sensor_msgs
cv_bridge
geometry_msgs
tf2
tf2_ros
)

ament_target_dependencies(
morphological_transformations
rclcpp
sensor_msgs
cv_bridge
geometry_msgs
tf2
tf2_ros
)


target_include_directories(color_and_segmentation PUBLIC
  ${OpenCV_INCLUDE_DIRS}
)

target_include_directories(morphological_transformations PUBLIC
  ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(color_and_segmentation ${OpenCV_LIBRARIES})
target_link_libraries(morphological_transformations ${OpenCV_LIBRARIES})



INSTALL(TARGETS
color_and_segmentation
morphological_transformations
DESTINATION lib/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
