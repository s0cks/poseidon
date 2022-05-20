find_package(Threads REQUIRED)
find_package(Glog REQUIRED)
find_package(gflags REQUIRED)

configure_file(poseidon/poseidon.h.in ${PROJECT_BINARY_DIR}/poseidon/poseidon.h)
include_directories(${PROJECT_BINARY_DIR})

add_library(poseidon
    ${PROJECT_BINARY_DIR}/poseidon/poseidon.h poseidon/poseidon.cc
    poseidon/platform/platform.h
    poseidon/common.h
    poseidon/utils.h
    poseidon/heap/heap.h poseidon/heap/heap.cc
    poseidon/local.h poseidon/local.cc
    poseidon/allocator.h poseidon/allocator.cc
    poseidon/raw_object.h poseidon/raw_object.cc
    poseidon/scavenger.h poseidon/scavenger.cc
    poseidon/marker.h poseidon/marker.cc
    poseidon/finalizer.h
    poseidon/platform/memory_region.h poseidon/platform/memory_region.cc  poseidon/platform/memory_region_linux.cc poseidon/platform/memory_region_win.cc poseidon/platform/memory_region_osx.cc
    poseidon/heap/zone.h poseidon/heap/zone.cc
    poseidon/heap/semispace.h
    poseidon/flags.h
    poseidon/relaxed_atomic.h
    poseidon/compactor.h poseidon/compactor.cc
    poseidon/task_pool.h poseidon/task_pool.cc
    poseidon/wsq.h
    poseidon/platform/os_thread.h
    poseidon/platform/os_thread_linux.h poseidon/platform/os_thread_linux.cc
    poseidon/platform/os_thread_osx.h poseidon/platform/os_thread_osx.cc
    poseidon/bitset.h
    poseidon/freelist.h poseidon/freelist.cc
    poseidon/sweeper.h poseidon/sweeper.cc
    poseidon/runtime.h poseidon/runtime.cc
    poseidon/heap/zone_new.h poseidon/heap/zone_new.cc
    poseidon/heap/zone_old.h poseidon/heap/zone_old.cc
    poseidon/utils.cc poseidon/finalizer.cc)
target_link_libraries(poseidon
    ${CMAKE_THREAD_LIBS_INIT}
    glog::glog
    ${GFLAGS_LIBRARIES})
target_include_directories(poseidon PUBLIC ${PROJECT_SOURCE_DIR}/Sources ${PROJECT_BINARY_DIR})