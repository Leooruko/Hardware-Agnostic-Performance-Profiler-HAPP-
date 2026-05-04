cmake_minimum_required(VERSION 3.10)
project(HAPP)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_executable(happ src/main.cpp src/hardware_profile.cpp src/hardware_profiles.cpp)