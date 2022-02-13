find_package(Threads REQUIRED)
find_package(Glog REQUIRED)
find_package(gflags REQUIRED)

configure_file(poseidon.h.in ${PROJECT_BINARY_DIR}/poseidon.h)
include_directories(${PROJECT_BINARY_DIR})

add_library(poseidon
    ${PROJECT_BINARY_DIR}/poseidon.h poseidon.cc
    platform.h
    common.h
    utils.h
    object.h object.cc
    heap.h heap.cc
    raw_object_printer.h
    local.h local.cc
    allocator.h allocator.cc
    raw_object.h raw_object.cc
    scavenger.h scavenger.cc
    heap_printer.h
        finalizer.h
    memory_region.h memory_region.cc memory_region_linux.cc memory_region_win.cc memory_region_osx.cc)
target_link_libraries(poseidon
    ${CMAKE_THREAD_LIBS_INIT}
    glog::glog
    ${GFLAGS_LIBRARIES})
target_include_directories(poseidon PUBLIC ${PROJECT_SOURCE_DIR}/Sources ${PROJECT_BINARY_DIR})