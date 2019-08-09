cmake_minimum_required(VERSION 3.14)
project(tinyhttp)

set(CMAKE_CXX_STANDARD 17)

add_library(tinyhttp SHARED src/library.cpp src/library.hpp)
if(WIN32)
    target_link_libraries(tinyhttp wsock32 ws2_32)
endif()