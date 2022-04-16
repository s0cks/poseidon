find_package(GTest REQUIRED CONFIG)
find_package(Threads REQUIRED)
find_package(Glog REQUIRED)

add_executable(poseidon-tests
    main.cc helpers.h)
target_link_libraries(poseidon-tests
    poseidon
    ${CMAKE_THREAD_LIBS_INIT}
    GTest::gtest
    GTest::gmock
    glog::glog)

include(GoogleTest)
