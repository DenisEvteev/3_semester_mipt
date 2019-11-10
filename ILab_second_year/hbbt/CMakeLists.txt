cmake_minimum_required(VERSION 3.14)
project(BST)

set(CMAKE_CXX_STANDARD 17)

set(My_Allocator TrackingAllocator.hpp)
add_executable(BST main.cpp ${My_Allocator} AVL_Tree.hpp AVL_Tree.h)