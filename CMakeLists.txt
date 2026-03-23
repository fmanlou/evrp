cmake_minimum_required(VERSION 3.14)
project(evrp LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Lua 5.4 (git subtree at third_party/lua)
add_subdirectory(third_party/lua)

add_subdirectory(third_party/googletest)
enable_testing()

add_library(evdev STATIC evdev.cpp)
target_include_directories(evdev PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(filesystem STATIC filesystem.cpp)
target_include_directories(filesystem PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(evdev PRIVATE filesystem)

add_library(deviceid STATIC deviceid.cpp)
target_include_directories(deviceid PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

option(EVRP_USE_X11_CURSOR "Enable X11 cursor position backend" ON)

set(cursorpos_sources cursor/cursorpos.cpp)
if(EVRP_USE_X11_CURSOR)
  find_package(X11 REQUIRED)
  list(APPEND cursorpos_sources cursor/cursorposx11.cpp)
endif()

add_library(cursorpos STATIC ${cursorpos_sources})
target_include_directories(cursorpos PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
                                            ${CMAKE_CURRENT_SOURCE_DIR}/cursor)
if(EVRP_USE_X11_CURSOR)
  target_compile_definitions(cursorpos PUBLIC EVRP_USE_X11_CURSOR)
  target_link_libraries(cursorpos PUBLIC X11::X11)
endif()

add_library(argparser STATIC argparser.cpp)
target_include_directories(argparser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(argparser PRIVATE deviceid logger)

add_library(
  inputeventwriter STATIC inputeventwriter.cpp keyboard/keyboardeventwriter.cpp
                          mouse/mouseeventwriter.cpp)
target_include_directories(inputeventwriter PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(
  inputeventwriter
  PRIVATE deviceid
          evdev
          eventformat
          filesystem
          inputdevice
          logger
          cursorpos)

add_library(eventformat STATIC eventformat.cpp)
target_include_directories(eventformat PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(eventformat PRIVATE deviceid evdev inputdevice)

add_library(inputdevice STATIC inputdevice.cpp keyboard/keyboarddevice.cpp
                               touchdevice.cpp)
target_include_directories(inputdevice PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(inputdevice PUBLIC deviceid evdev filesystem)

add_library(record STATIC record.cpp)
target_include_directories(record PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(record PRIVATE evdev eventformat filesystem inputdevice
                                     logger)

add_library(logger STATIC logger.cpp)
target_include_directories(logger PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(playback STATIC playback.cpp)
target_include_directories(playback PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(
  playback
  PRIVATE evdev
          eventformat
          inputeventwriter
          inputdevice
          filesystem
          logger
          lua_bindings)

add_library(lua_bindings STATIC lua/lua_bindings.cpp)
target_include_directories(lua_bindings PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(lua_bindings PUBLIC lua inputeventwriter filesystem
                                          logger cursorpos)

add_executable(evrp main.cpp)
target_link_libraries(
  evrp
  PRIVATE inputdevice
          evdev
          filesystem
          argparser
          playback
          record
          lua_bindings
          logger
          cursorpos)

add_executable(evrp_tests tests/inputdevice_test.cpp tests/argparser_test.cpp)
target_link_libraries(evrp_tests PRIVATE inputdevice argparser logger
                                         GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(evrp_tests)

# ---------------------------------------------------------------------------
# evrp-device：gRPC 服务端（可选，需系统安装 gRPC C++ / protobuf / grpc_cpp_plugin）
# ---------------------------------------------------------------------------
option(EVRP_BUILD_DEVICE "Build evrp-device gRPC server (needs gRPC C++, protobuf)" OFF)

if(EVRP_BUILD_DEVICE)
  find_package(Protobuf REQUIRED)
  find_package(gRPC CONFIG REQUIRED)

  set(PROTO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/proto")
  set(DEVICE_PROTO "${PROTO_ROOT}/evrp/device/v1/device.proto")
  set(GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")

  find_program(GRPC_CPP_PLUGIN_PROGRAM NAMES grpc_cpp_plugin REQUIRED)
  find_program(PROTOC_PROGRAM NAMES protoc REQUIRED)

  set(DEVICE_PB_CC "${GEN_DIR}/evrp/device/v1/device.pb.cc")
  set(DEVICE_PB_H "${GEN_DIR}/evrp/device/v1/device.pb.h")
  set(DEVICE_GRPC_CC "${GEN_DIR}/evrp/device/v1/device.grpc.pb.cc")
  set(DEVICE_GRPC_H "${GEN_DIR}/evrp/device/v1/device.grpc.pb.h")

  add_custom_command(
    OUTPUT ${DEVICE_PB_CC} ${DEVICE_PB_H} ${DEVICE_GRPC_CC} ${DEVICE_GRPC_H}
    COMMAND ${CMAKE_COMMAND} -E make_directory "${GEN_DIR}/evrp/device/v1"
    COMMAND
      ${PROTOC_PROGRAM} --cpp_out=${GEN_DIR} --grpc_out=${GEN_DIR}
      --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN_PROGRAM} -I${PROTO_ROOT}
      ${DEVICE_PROTO}
    DEPENDS ${DEVICE_PROTO}
    COMMENT "Generating protobuf/gRPC sources from device.proto"
    VERBATIM)

  add_library(evrp_device_proto STATIC ${DEVICE_PB_CC} ${DEVICE_GRPC_CC})
  target_include_directories(evrp_device_proto PUBLIC ${GEN_DIR})
  target_link_libraries(evrp_device_proto PUBLIC protobuf::libprotobuf gRPC::grpc++)

  add_executable(evrp-device evrp-device/main.cpp evrp-device/device_service.cpp)
  target_include_directories(evrp-device PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/evrp-device)
  target_link_libraries(evrp-device PRIVATE evrp_device_proto gRPC::grpc++)
endif()
