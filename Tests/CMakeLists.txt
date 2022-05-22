find_package(GTest REQUIRED)
find_package(Threads REQUIRED)
find_package(Glog REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(poseidon-tests
    main.cc
    helpers.h
    test_raw_object.cc
        test_free_list.cc
        heap/test_semispace.cc
    collector/test_collector.cc heap/test_semispace.h heap/test_new_zone.cc heap/test_new_zone.h heap/test_old_page.cc heap/test_old_page.h memory_region_test.h collector/test_sweeper.cc collector/test_sweeper.h helpers/assertions.h)
target_link_libraries(poseidon-tests
    poseidon
    ${CMAKE_THREAD_LIBS_INIT}
    GTest::gtest
    GTest::gmock
    glog::glog)