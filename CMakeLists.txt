cmake_minimum_required(VERSION 3.1)
project(poseidon)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/")

if(CMAKE_BUILD_TYPE MATCHES Debug)
    message(STATUS "Compiling with debug mode.")
    add_compile_definitions(PSDN_DEBUG)
else()
    message(STATUS "Compiling without debug mode.")
endif()

set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_VERSION_MINOR 0)
set(PROJECT_VERSION_PATCH 0)

add_subdirectory(Tests)
add_subdirectory(Sources)
