cmake_minimum_required(VERSION 2.8)

project(re)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)


add_subdirectory(vendor/fmt EXCLUDE_FROM_ALL)

# Include headers
include_directories (include)
include_directories ("vendor/fmt")

# Source codes (except main.cpp)
file (GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(re ${SOURCES})

target_link_libraries(re PRIVATE fmt::fmt-header-only)
