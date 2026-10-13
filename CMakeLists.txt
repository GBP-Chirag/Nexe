cmake_minimum_required(VERSION 3.15)
project(postake_core CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directory for header files
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Compile the high-performance C++ backend engine
add_executable(postake_engine
    src/main.cpp
    src/dag.cpp
    src/hasher.cpp
)