find_package(Threads REQUIRED)
find_package(Glog REQUIRED)
find_package(gflags REQUIRED)

configure_file(poseidon/poseidon.h.in ${PROJECT_BINARY_DIR}/poseidon/poseidon.h)
include_directories(${PROJECT_BINARY_DIR})

add_library(poseidon
    ${PROJECT_BINARY_DIR}/poseidon/poseidon.h poseidon/poseidon.cc
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
    poseidon/heap/old_page.h
    poseidon/heap/semispace.h poseidon/heap/semispace.cc
    poseidon/heap/zone.h
    poseidon/heap/new_zone.h poseidon/heap/new_zone.cc
    poseidon/heap/old_zone.h poseidon/heap/old_zone.cc
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
    poseidon/heap/freelist.h poseidon/heap/freelist.cc
    poseidon/local.h poseidon/local.cc
    poseidon/raw_object.h poseidon/raw_object.cc
    poseidon/relaxed_atomic.h
    poseidon/runtime.h poseidon/runtime.cc
    poseidon/task_pool.h poseidon/task_pool.cc
    poseidon/utils.h poseidon/utils.cc
    poseidon/wsq.h poseidon/heap/section.cc poseidon/heap/page.h poseidon/reference.h poseidon/reference.cc poseidon/frontend/instruction.h poseidon/frontend/store_instruction.h poseidon/frontend/load_instruction.h poseidon/frontend/graph_entry_instr.h poseidon/frontend/block_entry_instr.h poseidon/heap/page_tag.h poseidon/heap/page_table.h poseidon/heap/page_table.cc poseidon/heap/new_page.h poseidon/heap/new_page.cc poseidon/heap/page.cc poseidon/heap/page_marker.h poseidon/region.h poseidon/heap/zone.cc poseidon/heap/old_page.cc poseidon/heap/freelist_node.h poseidon/sweeper/sweeper_stats.h poseidon/sweeper/sweeper_base.h poseidon/sweeper/sweeper_serial.h poseidon/sweeper/sweeper_parallel.h poseidon/marker/marker.h poseidon/marker/marker_visitor.h poseidon/marker/marker_serial.h poseidon/marker/marker.cc poseidon/sweeper/sweeper.cc poseidon/sweeper/sweeper.h poseidon/sweeper/sweeper_serial.cc poseidon/heap/freelist_printer.h poseidon/object_tag.cc poseidon/object_tag.h)
target_link_libraries(poseidon
    ${CMAKE_THREAD_LIBS_INIT}
    glog::glog
    ${GFLAGS_LIBRARIES})
target_include_directories(poseidon PUBLIC ${PROJECT_SOURCE_DIR}/Sources ${PROJECT_BINARY_DIR})