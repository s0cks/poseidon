FIND_PACKAGE(GTest REQUIRED)
FIND_PACKAGE(Threads REQUIRED)
FIND_PACKAGE(Glog REQUIRED)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

ENABLE_TESTING()

ADD_EXECUTABLE(poseidon-tests
               main.cc
               helpers.h
               test_raw_object.cc
               heap/test_semispace.cc
               zone/test_new_zone.cc
               heap/test_old_page.cc
               helpers/assertions.h
               heap/test_new_page.cc
               zone/test_old_zone.cc
               heap/test_free_object.cc
               marker/test_marker_serial.cc
               sweeper/test_sweeper_serial.cc
               test_pointer.cc
               test_pointer_tag.cc
               heap/mock_new_page_visitor.h
               heap/mock_old_page_visitor.h
               helpers/alloc_helpers.h
               heap/test_heap.cc
               sweeper/mock_sweeper.h
               marker/mock_marker.h
               scavenger/test_scavenger_serial.cc
               scavenger/mock_scavenger.h
               type/test_bool.cc
               local/test_local_page.cc
               matchers/is_pointer_to.h
               matchers/object_tag_matcher.h
               type/test_tuple.cc
               assertions/assertions.h
               assertions/pointers_eq.h
               assertions/type_assertions.h
               mock_raw_object_visitor.h
               page/test_page_tag.cc
               helpers/type_alloc_helpers.h
               utils/test_size.cc
               page/test_page_table.cc
               freelist/test_freelist.h
               freelist/test_freelist.cc
               freelist/mock_free_list.h
               freelist/mock_free_pointer.h
               freelist/is_free_ptr_to.h type/test_uint8.cc assertions/ptr_assertions.h marker/test_marker_parallel.cc zone/test_new_zone.h zone/test_new_zone_try_allocate.cc page/test_new_page.cc page/test_new_page.h base_memory_region_test.h zone/test_new_zone_visit_pointers.cc zone/test_new_zone_visit_marked_pointers.cc)
TARGET_LINK_LIBRARIES(poseidon-tests
                      poseidon
                      ${CMAKE_THREAD_LIBS_INIT}
                      GTest::gtest
                      GTest::gmock
                      glog::glog)
include(GoogleTest)
# GTEST_DISCOVER_TESTS(poseidon-tests)