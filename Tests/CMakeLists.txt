find_package(GTest REQUIRED)
find_package(Threads REQUIRED)
find_package(Glog REQUIRED)

add_executable(poseidon-tests
    main.cc test_compactor.cc helpers.h test_scavenger.cc mock_zone.h test_allocator.cc test_heap.cc)
target_link_libraries(poseidon-tests
    poseidon
    ${CMAKE_THREAD_LIBS_INIT}
    GTest::gtest
    GTest::gmock
    glog::glog)