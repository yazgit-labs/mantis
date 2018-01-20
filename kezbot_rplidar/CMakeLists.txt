cmake_minimum_required(VERSION 2.8.3)
project(kezbot_rplidar)

add_compile_options(-std=c++11)

set(RPLIDAR_SDK_PATH "./include/rplidar_sdk")

FILE(GLOB RPLIDAR_SDK_SRC
  "${RPLIDAR_SDK_PATH}/src/arch/linux/*.cpp"
  "${RPLIDAR_SDK_PATH}/src/hal/*.cpp"
  "${RPLIDAR_SDK_PATH}/src/*.cpp"
)

find_package(catkin REQUIRED COMPONENTS
  geometry_msgs
  nav_msgs
  roscpp
  rospy
  sensor_msgs
  std_msgs
)

include_directories(
  ${RPLIDAR_SDK_PATH}/include
  ${RPLIDAR_SDK_PATH}/src
  ${catkin_INCLUDE_DIRS}
)

catkin_package(
  INCLUDE_DIRS include
  CATKIN_DEPENDS geometry_msgs nav_msgs roscpp rospy sensor_msgs std_msgs
  )

add_executable(kezbot_rplidar_node src/rplidar_node.cpp ${RPLIDAR_SDK_SRC})
target_link_libraries(kezbot_rplidar_node ${catkin_LIBRARIES})
