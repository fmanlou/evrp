cmake_minimum_required(VERSION 3.14)
project(evrp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(third_party/googletest)
enable_testing()

add_library(input_device STATIC input_device.cpp)
target_include_directories(input_device PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(evrp main.cpp)
target_link_libraries(evrp PRIVATE input_device)

add_executable(evrp_tests tests/input_device_test.cpp)
target_link_libraries(evrp_tests PRIVATE input_device GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(evrp_tests)
