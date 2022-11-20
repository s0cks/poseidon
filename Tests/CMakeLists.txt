find_package(GTest REQUIRED)
find_package(Threads REQUIRED)
find_package(Glog REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(poseidon-tests
    main.cc
    helpers.h
    test_raw_object.cc
    heap/test_semispace.cc
    heap/test_new_zone.cc heap/test_old_page.cc helpers/assertions.h heap/test_new_page.cc heap/test_old_zone.cc heap/test_freelist.cc heap/test_free_object.cc marker/test_marker_serial.cc sweeper/test_sweeper_serial.cc test_object_tag.cc heap/mock_new_page_visitor.h heap/mock_old_page_visitor.h helpers/alloc_helpers.h heap/test_heap.cc sweeper/mock_sweeper.h marker/mock_marker.h scavenger/test_scavenger_serial.cc scavenger/mock_scavenger.h type/test_bool.cc type/test_byte.cc type/test_short.cc type/test_int.cc type/test_long.cc local/test_local_page.cc heap/semispace_printer.h matchers/is_pointer_to.h matchers/object_tag_matcher.h type/test_tuple.cc assertions/assertions.h assertions/pointers_eq.h assertions/int_assertions.h mock_raw_object_visitor.h)
target_link_libraries(poseidon-tests
    poseidon
    ${CMAKE_THREAD_LIBS_INIT}
    GTest::gtest
    GTest::gmock
    glog::glog)