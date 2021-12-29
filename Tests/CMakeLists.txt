find_package(GTest REQUIRED)
find_package(Threads REQUIRED)
find_package(Glog REQUIRED)

add_executable(poseidon-tests
    main.cc)
target_link_libraries(poseidon-tests
    poseidon
    ${CMAKE_THREAD_LIBS_INIT}
    GTest::GTest
    glog::glog)