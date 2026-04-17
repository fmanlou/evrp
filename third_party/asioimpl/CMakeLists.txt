cmake_minimum_required(VERSION 3.14)
project(evrp_asio_static LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(asio_separate STATIC asioimpl.cpp)
target_include_directories(asio_separate PRIVATE "${CMAKE_INSTALL_PREFIX}/include")
target_compile_definitions(asio_separate PRIVATE ASIO_STANDALONE ASIO_SEPARATE_COMPILATION)
find_package(Threads REQUIRED)
target_link_libraries(asio_separate PUBLIC Threads::Threads)

install(TARGETS asio_separate ARCHIVE DESTINATION lib)
