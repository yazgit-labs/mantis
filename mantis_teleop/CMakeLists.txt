cmake_minimum_required(VERSION 2.8.3)
project(mantis_teleop)

add_compile_options(-std=c++11) 

find_package(catkin REQUIRED COMPONENTS
  geometry_msgs
  joy
  nav_msgs
  roscpp
)

catkin_package()

include_directories(
 ${catkin_INCLUDE_DIRS}
)

add_executable(mantis_teleop_joy_node src/teleop_joy.cpp)
target_link_libraries(mantis_teleop_joy_node ${catkin_LIBRARIES})
