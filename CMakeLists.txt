cmake_minimum_required(VERSION 3.15)
project(Nexe_core LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# CMake selects the compiler/target; NEXE_TARGET_ARCH records and validates the
# intended output architecture. For cross builds, pass the matching toolchain:
#   cmake -S . -B build-aarch64 -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/linux-aarch64.cmake
set(NEXE_TARGET_ARCH "native" CACHE STRING "Target architecture: native, amd64, or aarch64")
set_property(CACHE NEXE_TARGET_ARCH PROPERTY STRINGS native amd64 aarch64)
string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" NEXE_SYSTEM_PROCESSOR)
if(NEXE_TARGET_ARCH STREQUAL "native")
    if(NEXE_SYSTEM_PROCESSOR MATCHES "^(x86_64|amd64|x64)$")
        set(NEXE_RESOLVED_ARCH "amd64")
    elseif(NEXE_SYSTEM_PROCESSOR MATCHES "^(aarch64|arm64)$")
        set(NEXE_RESOLVED_ARCH "aarch64")
    else()
        message(FATAL_ERROR "Unsupported native architecture '${CMAKE_SYSTEM_PROCESSOR}'. Use amd64 or aarch64.")
    endif()
elseif(NEXE_TARGET_ARCH STREQUAL "amd64" OR NEXE_TARGET_ARCH STREQUAL "aarch64")
    set(NEXE_RESOLVED_ARCH "${NEXE_TARGET_ARCH}")
else()
    message(FATAL_ERROR "NEXE_TARGET_ARCH must be native, amd64, or aarch64.")
endif()
message(STATUS "Nexe target: ${CMAKE_SYSTEM_NAME} ${NEXE_RESOLVED_ARCH}")

if(NOT DEFINED SRC_LIST OR "${SRC_LIST}" STREQUAL "")
    unset(SRC_LIST)
    file(GLOB_RECURSE SRC_LIST CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cxx")
endif()

if(NOT SRC_LIST OR "${SRC_LIST}" STREQUAL "")
    message(FATAL_ERROR "No source files found for nexe_engine. Add sources to SRC_LIST or create files under ${CMAKE_CURRENT_SOURCE_DIR}/src.")
endif()

add_executable(nexe_engine ${SRC_LIST}
    Nexe_CLI/parser.h)

target_include_directories(nexe_engine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_definitions(nexe_engine PRIVATE NEXE_TARGET_ARCH="${NEXE_RESOLVED_ARCH}")

if(MSVC)
    target_compile_options(nexe_engine PRIVATE /W4 /permissive-)
else()
    target_compile_options(nexe_engine PRIVATE -Wall -Wextra -Wpedantic)
endif()
