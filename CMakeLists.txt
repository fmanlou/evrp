cmake_minimum_required(VERSION 3.14)
project(evrp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(third_party/googletest)
enable_testing()

add_library(evdev STATIC evdev/evdevlinux.cpp)
target_include_directories(evdev PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(filesystem STATIC filesystem/filesystem.cpp)
target_include_directories(filesystem PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(evdev PRIVATE filesystem)

add_library(argparser STATIC argparser.cpp)
target_include_directories(argparser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(inputdevice STATIC inputdevice.cpp keyboarddevice.cpp touchdevice.cpp)
target_include_directories(inputdevice PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(inputdevice PUBLIC evdev filesystem)

add_library(record STATIC record.cpp)
target_include_directories(record PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(record PRIVATE evdev filesystem inputdevice)

add_library(asynclogwriter STATIC asynclogwriter.cpp)
target_include_directories(asynclogwriter PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(playback STATIC playback.cpp)
target_include_directories(playback PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(playback PUBLIC asynclogwriter PRIVATE evdev inputdevice filesystem)

add_executable(evrp main.cpp)
target_link_libraries(evrp PRIVATE inputdevice evdev filesystem argparser playback record)

add_executable(evrp_tests tests/inputdevice_test.cpp tests/argparser_test.cpp)
target_link_libraries(evrp_tests PRIVATE inputdevice argparser GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(evrp_tests)
