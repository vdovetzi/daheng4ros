cmake_minimum_required(VERSION 3.8)
project(daheng4ros)

# Set C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(image_transport REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(OpenCV REQUIRED)
find_package(rcl_interfaces REQUIRED)

####### Daheng lib #############
add_library(daheng_cam SHARED 
  src/camera.cpp
)

target_include_directories(daheng_cam PUBLIC
  include
)

target_link_libraries(daheng_cam PUBLIC
 gxiapi
)
###############################

add_executable(image_publisher src/image_publisher.cpp)
target_link_libraries(image_publisher 
  daheng_cam
  ${OpenCV_LIBS}
)
ament_target_dependencies(image_publisher
  rclcpp
  sensor_msgs
  image_transport
  cv_bridge
  rcl_interfaces
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

install(TARGETS
  image_publisher
  DESTINATION lib/${PROJECT_NAME}/
)

install(
  DIRECTORY launch/
  DESTINATION share/${PROJECT_NAME}/
)

install(
  DIRECTORY config/
  DESTINATION share/${PROJECT_NAME}/
)

ament_package()
