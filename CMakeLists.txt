cmake_minimum_required(VERSION 3.10)

project(mini_os VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(include)

add_executable(mini_os src/main.cpp)