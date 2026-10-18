cmake_minimum_required(VERSION 3.15)
project(Nexe_core LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(nexe_engine
    src/main.cpp
    src/dag.cpp
    src/hasher.cpp
)
target_include_directories(nexe_engine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(MSVC)
    target_compile_options(nexe_engine PRIVATE /W4 /permissive-)
else()
    target_compile_options(nexe_engine PRIVATE -Wall -Wextra -Wpedantic)
endif()
