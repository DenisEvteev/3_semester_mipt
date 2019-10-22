cmake_minimum_required(VERSION 3.14)
project(intersect)

set(CMAKE_CXX_STANDARD 14)

set(Pivotal_Class_For_Polyogns Work_w_Polygon.hpp Work_w_Polygon.h)

set(BSPSource BSPTree.hpp BSPTree.h)

set(line_class line_2d.hpp line_2d.h)

add_executable(intersect main.cpp ${Pivotal_Class_For_Polygons} ${BSPSource} ${line_class})