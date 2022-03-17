find_package(Threads REQUIRED)
find_package(Glog REQUIRED)
find_package(gflags REQUIRED)

configure_file(poseidon/poseidon.h.in ${PROJECT_BINARY_DIR}/poseidon/poseidon.h)
include_directories(${PROJECT_BINARY_DIR})

add_library(poseidon
    ${PROJECT_BINARY_DIR}/poseidon/poseidon.h poseidon/poseidon.cc
    poseidon/platform.h
    poseidon/common.h
    poseidon/utils.h
    poseidon/heap.h poseidon/heap.cc
    poseidon/local.h
    poseidon/allocator.h poseidon/allocator.cc
    poseidon/raw_object.h poseidon/raw_object.cc
    poseidon/scavenger.h poseidon/scavenger.cc
    poseidon/finalizer.h
    poseidon/memory_region.h poseidon/memory_region_linux.cc poseidon/memory_region_win.cc poseidon/memory_region_osx.cc poseidon/zone.h poseidon/semispace.h poseidon/flags.h poseidon/relaxed_atomic.h poseidon/compactor.cc poseidon/compactor.h poseidon/task_pool.h poseidon/wsq.h poseidon/os_thread.h poseidon/os_thread_linux.h poseidon/os_thread_osx.h poseidon/os_thread_linux.cc poseidon/os_thread_osx.cc poseidon/task_pool.cc poseidon/bitset.h poseidon/zone.cc)
target_link_libraries(poseidon
    ${CMAKE_THREAD_LIBS_INIT}
    glog::glog
    ${GFLAGS_LIBRARIES})
target_include_directories(poseidon PUBLIC ${PROJECT_SOURCE_DIR}/Sources ${PROJECT_BINARY_DIR})