cmake_minimum_required(VERSION 3.21)
project(poseidon)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/")

if(CMAKE_BUILD_TYPE MATCHES Debug)
    message(STATUS "Compiling with debug mode.")
    add_compile_definitions(POSEIDON_DEBUG)
    add_compile_definitions(PSDN_DEBUG)
else()
    message(STATUS "Compiling without debug mode.")
endif()

option(PSDN_MTA "Enable faster heap collections using multi-threaded algorithm [Experimental]" OFF)
if(PSDN_MTA)
    message(STATUS "Compiling w/ MTA")
    add_compile_definitions(PSDN_MTA)
endif()

option(PSDN_ENABLE_GCLOGS "Enable the garbage collector logs. Disabled by default, degrades performance due to noise." OFF)
if(PSDN_ENABLE_GCLOGS)
    message(STATUS "Compiling w/ Garbage Collector Logs")
    add_compile_definitions(PSDN_GCLOGS)
endif()

set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_VERSION_MINOR 0)
set(PROJECT_VERSION_PATCH 0)

add_subdirectory(Tests)
add_subdirectory(Sources)