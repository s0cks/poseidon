FIND_PACKAGE(Threads REQUIRED)
FIND_PACKAGE(glog REQUIRED)
FIND_PACKAGE(gflags REQUIRED)

CONFIGURE_FILE(poseidon/poseidon.h.in ${PROJECT_BINARY_DIR}/poseidon/poseidon.h)
INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR})

SET(PSDN_HEAP_PAGE_SOURCES
    poseidon/page/page_tag.h
    poseidon/page/page.h
    poseidon/page/page_table.h
    poseidon/page/page_table.cc
    poseidon/page/new_page.h
    poseidon/page/new_page.cc
    poseidon/page/old_page.h
    poseidon/page/old_page.cc)
SET(PSDN_HEAP_ZONE_SOURCES
    poseidon/zone/zone.h
    poseidon/zone/new_zone.h
    poseidon/zone/new_zone.cc
    poseidon/zone/old_zone.h
    poseidon/zone/old_zone.cc poseidon/flags.cc poseidon/utils/size.h)

ADD_LIBRARY(poseidon
            ${PROJECT_BINARY_DIR}/poseidon/poseidon.h poseidon/poseidon.cc
            ${PSDN_HEAP_PAGE_SOURCES}
            ${PSDN_HEAP_ZONE_SOURCES}
            # allocator
            poseidon/allocator/allocator.h poseidon/allocator/allocator.cc
            # collector
            poseidon/collector/collector.h poseidon/collector/collector.cc
            poseidon/collector/compactor.h poseidon/collector/compactor.cc
            poseidon/collector/finalizer.h poseidon/collector/finalizer.cc
            poseidon/collector/scavenger.h poseidon/collector/scavenger.cc
            # heap
            poseidon/heap/heap.h poseidon/heap/heap.cc
            poseidon/heap/section.h
            poseidon/heap/semispace.h poseidon/heap/semispace.cc
            # platform
            poseidon/platform/memory_region.h poseidon/platform/memory_region.cc
            poseidon/platform/memory_region_linux.cc
            poseidon/platform/memory_region_win.cc
            poseidon/platform/memory_region_osx.cc
            poseidon/platform/os_thread.h
            poseidon/platform/os_thread_linux.h poseidon/platform/os_thread_linux.cc
            poseidon/platform/os_thread_osx.h poseidon/platform/os_thread_osx.cc
            poseidon/platform/platform.h
            # core
            poseidon/bitset.h
            poseidon/common.h
            poseidon/flags.h
            poseidon/freelist/freelist.h poseidon/freelist/freelist.cc
            poseidon/pointer.h poseidon/pointer.cc
            poseidon/relaxed_atomic.h
            poseidon/runtime.h poseidon/runtime.cc
            poseidon/task/task_pool.h poseidon/task/task_pool.cc
            poseidon/utils.h poseidon/utils.cc
            poseidon/wsq.h poseidon/heap/section.cc
            poseidon/region.h
            poseidon/freelist/free_ptr.h
            poseidon/sweeper/sweeper_stats.h
            poseidon/sweeper/sweeper_visitor.h
            poseidon/sweeper/sweeper_serial.h
            poseidon/sweeper/sweeper_parallel.h
            poseidon/marker/marker.h
            poseidon/marker/marker_visitor.h
            poseidon/marker/marker_serial.h
            poseidon/marker/marker.cc
            poseidon/sweeper/sweeper.cc
            poseidon/sweeper/sweeper.h
            poseidon/sweeper/sweeper_serial.cc
            poseidon/pointer_tag.h
            poseidon/freelist/free_ptr.cc
            poseidon/marker/marker_serial.cc
            poseidon/heap/page_marker.h
            poseidon/scavenger/scavenger.cc
            poseidon/scavenger/scavenger.h
            poseidon/scavenger/scavenger_visitor.h
            poseidon/scavenger/scavenger_serial.cc
            poseidon/scavenger/scavenger_serial.h
            poseidon/marker/marker_stats.h
            poseidon/pointer_counter.h
            poseidon/local/local_page.h
            poseidon/local/local_page.cc
            poseidon/local/local.h
            poseidon/freelist/freelist_printer.h poseidon/array.h poseidon/utils/visitor.h poseidon/object.h poseidon/object.cc poseidon/type.h poseidon/marker/marker_parallel.h poseidon/marker/marker_parallel.cc poseidon/task/task.h poseidon/task/task.cc poseidon/task/task_worker.h poseidon/task/task_worker.cc poseidon/platform/os_thread.cc)
TARGET_LINK_LIBRARIES(poseidon
                      ${CMAKE_THREAD_LIBS_INIT}
                      glog::glog
                      ${GFLAGS_LIBRARIES})
TARGET_INCLUDE_DIRECTORIES(poseidon PUBLIC ${PROJECT_SOURCE_DIR}/Sources ${PROJECT_BINARY_DIR})