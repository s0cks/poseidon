find_package(GTest REQUIRED)
find_package(Threads REQUIRED)
find_package(Glog REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(poseidon-tests
    main.cc
    helpers.h
    test_raw_object.cc
    heap/test_semispace.cc
    heap/test_new_zone.cc heap/test_old_page.cc helpers/assertions.h frontend/test_instruction.h frontend/test_instruction.cc heap/test_page_tag.cc heap/test_new_page.cc heap/test_old_zone.cc heap/test_freelist.cc heap/test_freelist_node.cc marker/test_marker_serial.cc sweeper/test_sweeper_serial.cc test_object_tag.cc)
target_link_libraries(poseidon-tests
    poseidon
    ${CMAKE_THREAD_LIBS_INIT}
    GTest::gtest
    GTest::gmock
    glog::glog)