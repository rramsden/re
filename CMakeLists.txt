cmake_minimum_required(VERSION 2.8)

project(re)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

include_directories("${PROJECT_SOURCE_DIR}")

add_executable(re ${PROJECT_SOURCE_DIR}/re.cpp)
