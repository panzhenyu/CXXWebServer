cmake_minimum_required(VERSION 3.10)

# set project name
project(WebServer)

# set c++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)

# set compiler flags
set(CXX_FLAGS
    -g
    -Wall
    -std=c++17
)
string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")

# add sub dir
add_subdirectory(src)
