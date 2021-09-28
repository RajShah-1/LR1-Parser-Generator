cmake_minimum_required(VERSION 3.20)
project(CD_6)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)
include_directories(src)
include_directories(src/Module_1)

add_executable(CD_6
        src/Module_1/LR1.cpp
        src/Module_1/main.cpp
        src/Module_1/SetOfItems.cpp)
