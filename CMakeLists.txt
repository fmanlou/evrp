cmake_minimum_required(VERSION 3.14)
project(evrp LANGUAGES C CXX)

# --- library/ 与
# Lua、GoogleTest、Protobuf、gRPC、gflags、fmt、spdlog、log（安装：scripts/install-third-party-to-library.sh）---
set(EVRP_LIBRARY_DIR
    "${CMAKE_SOURCE_DIR}/library"
    CACHE PATH "Third-party install prefix for this repo")
list(APPEND CMAKE_PREFIX_PATH "${EVRP_LIBRARY_DIR}")
list(APPEND CMAKE_PREFIX_PATH "${EVRP_LIBRARY_DIR}/protobuf")
list(APPEND CMAKE_PREFIX_PATH "${EVRP_LIBRARY_DIR}/grpc")
list(APPEND CMAKE_PREFIX_PATH "${EVRP_LIBRARY_DIR}/gflags")
list(APPEND CMAKE_PREFIX_PATH "${EVRP_LIBRARY_DIR}/fmt")
list(APPEND CMAKE_PREFIX_PATH "${EVRP_LIBRARY_DIR}/spdlog")
list(APPEND CMAKE_PREFIX_PATH "${EVRP_LIBRARY_DIR}/log")

find_library(
  EVRP_LUA_LIB
  NAMES lua liblua lua_static
  PATHS "${EVRP_LIBRARY_DIR}/lua/lib" "${EVRP_LIBRARY_DIR}/lua/lib64"
  NO_DEFAULT_PATH)
find_path(
  EVRP_LUA_INCLUDE_DIR
  NAMES lua.h
  PATHS "${EVRP_LIBRARY_DIR}/lua/include/lua"
  NO_DEFAULT_PATH)
if(NOT EVRP_LUA_LIB OR NOT EVRP_LUA_INCLUDE_DIR)
  message(
    FATAL_ERROR
      "Lua not found under '${EVRP_LIBRARY_DIR}/lua'. Run: ./scripts/install-third-party-to-library.sh"
  )
endif()
add_library(lua STATIC IMPORTED GLOBAL)
set_target_properties(lua PROPERTIES IMPORTED_LOCATION "${EVRP_LUA_LIB}")
target_include_directories(lua INTERFACE "${EVRP_LUA_INCLUDE_DIR}")
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_libraries(lua INTERFACE m)
endif()

find_package(GTest CONFIG REQUIRED PATHS "${EVRP_LIBRARY_DIR}/googletest"
             NO_DEFAULT_PATH)

# gflags（argparser、evrp-device）
set(GFLAGS_USE_TARGET_NAMESPACE ON)
find_package(gflags CONFIG REQUIRED PATHS "${EVRP_LIBRARY_DIR}/gflags"
             NO_DEFAULT_PATH)

# fmanlou/log（fmt + spdlog 依赖；PREFIX_PATH 已含 library/fmt、library/spdlog）
find_package(log CONFIG REQUIRED PATHS "${EVRP_LIBRARY_DIR}/log"
             NO_DEFAULT_PATH)

# Protobuf 5.x / gRPC 需要 C++17；其余源码与之兼容
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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
target_link_libraries(argparser PRIVATE deviceid logger gflags::gflags)

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
          luabindings)

add_library(luabindings STATIC lua/luabindings.cpp)
target_include_directories(luabindings PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(luabindings PUBLIC lua inputeventwriter filesystem logger
                                         cursorpos)

add_executable(evrp main.cpp)
target_link_libraries(
  evrp
  PRIVATE inputdevice
          evdev
          filesystem
          argparser
          playback
          record
          luabindings
          logger
          cursorpos)

add_executable(evrp_tests tests/inputdevice_test.cpp tests/argparser_test.cpp)
target_link_libraries(evrp_tests PRIVATE inputdevice argparser logger
                                         GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(evrp_tests)

# ---------------------------------------------------------------------------
# evrp-device：gRPC 服务端（依赖安装到 library/ 或系统 PATH，见 docs/PROJECT_CONVENTIONS.md）
# ---------------------------------------------------------------------------
# 勿在此单独先 find_package(Protobuf)：gRPC 的 gRPCConfig.cmake 会
# find_dependency(Protobuf)， 若重复加载 protobuf-targets.cmake 会报错「Some targets in
# this export set were already defined」。
find_package(gRPC CONFIG REQUIRED)

set(PROTO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/proto")
set(DEVICE_TYPES_PROTO "${PROTO_ROOT}/evrp/device/v1/types.proto")
set(DEVICE_INPUTLISTEN_PROTO "${PROTO_ROOT}/evrp/device/v1/service/inputlisten.proto")
set(DEVICE_PLAYBACK_PROTO "${PROTO_ROOT}/evrp/device/v1/service/playback.proto")
set(DEVICE_SERVICE_PROTO "${PROTO_ROOT}/evrp/device/v1/service/service.proto")
set(GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")

find_program(
  GRPC_CPP_PLUGIN_PROGRAM
  NAMES grpc_cpp_plugin REQUIRED
  HINTS "${EVRP_LIBRARY_DIR}/grpc/bin" "${EVRP_LIBRARY_DIR}/bin")
find_program(
  PROTOC_PROGRAM
  NAMES protoc REQUIRED
  HINTS "${EVRP_LIBRARY_DIR}/protobuf/bin" "${EVRP_LIBRARY_DIR}/bin")

set(DEVICE_TYPES_PB_CC "${GEN_DIR}/evrp/device/v1/types.pb.cc")
set(DEVICE_TYPES_PB_H "${GEN_DIR}/evrp/device/v1/types.pb.h")
set(DEVICE_INPUTLISTEN_PB_CC "${GEN_DIR}/evrp/device/v1/service/inputlisten.pb.cc")
set(DEVICE_INPUTLISTEN_PB_H "${GEN_DIR}/evrp/device/v1/service/inputlisten.pb.h")
set(DEVICE_INPUTLISTEN_GRPC_CC "${GEN_DIR}/evrp/device/v1/service/inputlisten.grpc.pb.cc")
set(DEVICE_INPUTLISTEN_GRPC_H "${GEN_DIR}/evrp/device/v1/service/inputlisten.grpc.pb.h")
set(DEVICE_PLAYBACK_PB_CC "${GEN_DIR}/evrp/device/v1/service/playback.pb.cc")
set(DEVICE_PLAYBACK_PB_H "${GEN_DIR}/evrp/device/v1/service/playback.pb.h")
set(DEVICE_PLAYBACK_GRPC_CC "${GEN_DIR}/evrp/device/v1/service/playback.grpc.pb.cc")
set(DEVICE_PLAYBACK_GRPC_H "${GEN_DIR}/evrp/device/v1/service/playback.grpc.pb.h")
set(DEVICE_SERVICE_PB_CC "${GEN_DIR}/evrp/device/v1/service/service.pb.cc")
set(DEVICE_SERVICE_PB_H "${GEN_DIR}/evrp/device/v1/service/service.pb.h")
set(DEVICE_SERVICE_GRPC_CC "${GEN_DIR}/evrp/device/v1/service/service.grpc.pb.cc")
set(DEVICE_SERVICE_GRPC_H "${GEN_DIR}/evrp/device/v1/service/service.grpc.pb.h")

add_custom_command(
  OUTPUT ${DEVICE_TYPES_PB_CC} ${DEVICE_TYPES_PB_H} ${DEVICE_INPUTLISTEN_PB_CC}
         ${DEVICE_INPUTLISTEN_PB_H} ${DEVICE_INPUTLISTEN_GRPC_CC}
         ${DEVICE_INPUTLISTEN_GRPC_H} ${DEVICE_PLAYBACK_PB_CC} ${DEVICE_PLAYBACK_PB_H}
         ${DEVICE_PLAYBACK_GRPC_CC} ${DEVICE_PLAYBACK_GRPC_H} ${DEVICE_SERVICE_PB_CC}
         ${DEVICE_SERVICE_PB_H} ${DEVICE_SERVICE_GRPC_CC} ${DEVICE_SERVICE_GRPC_H}
  COMMAND ${CMAKE_COMMAND} -E make_directory "${GEN_DIR}/evrp/device/v1/service"
  COMMAND
    ${PROTOC_PROGRAM} --cpp_out=${GEN_DIR} --grpc_out=${GEN_DIR}
    --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN_PROGRAM} -I${PROTO_ROOT}
    ${DEVICE_TYPES_PROTO} ${DEVICE_INPUTLISTEN_PROTO} ${DEVICE_PLAYBACK_PROTO}
    ${DEVICE_SERVICE_PROTO}
  DEPENDS ${DEVICE_TYPES_PROTO} ${DEVICE_INPUTLISTEN_PROTO} ${DEVICE_PLAYBACK_PROTO}
          ${DEVICE_SERVICE_PROTO}
  COMMENT
    "Generating protobuf/gRPC from types.proto and evrp/device/v1/service/*.proto"
  VERBATIM)

add_library(evrp_device_proto STATIC ${DEVICE_TYPES_PB_CC} ${DEVICE_INPUTLISTEN_PB_CC}
                                     ${DEVICE_INPUTLISTEN_GRPC_CC} ${DEVICE_PLAYBACK_PB_CC}
                                     ${DEVICE_PLAYBACK_GRPC_CC} ${DEVICE_SERVICE_PB_CC}
                                     ${DEVICE_SERVICE_GRPC_CC})
target_include_directories(evrp_device_proto PUBLIC ${GEN_DIR})
target_link_libraries(evrp_device_proto PUBLIC protobuf::libprotobuf
                                               gRPC::grpc++)

add_library(evrp_device_api INTERFACE)
target_include_directories(evrp_device_api
                           INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_library(countingsemaphore STATIC evrp/countingsemaphore.cpp)
target_include_directories(countingsemaphore
                           PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(
  countingsemaphore
  PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)

add_library(tofromproto STATIC evrp/device/internal/tofromproto.cpp)
target_include_directories(tofromproto
                           PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(tofromproto PUBLIC evrp_device_api evrp_device_proto)
set_target_properties(
  tofromproto
  PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)

add_library(
  localinputlistener STATIC evrp/device/server/localinputlistener.cpp
                           evrp/device/server/dispatchedinputlistener.cpp
                           evrp/device/server/syncdispatchqueue.cpp)
target_include_directories(localinputlistener
                           PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(
  localinputlistener
  PUBLIC evrp_device_api
  PRIVATE inputdevice)
set_target_properties(
  localinputlistener
  PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)

add_library(localplayback STATIC evrp/device/server/localplayback.cpp)
target_include_directories(localplayback PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(localplayback PUBLIC evrp_device_api inputeventwriter
                                           countingsemaphore)
set_target_properties(
  localplayback
  PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)

add_library(device_remote_inputlistener STATIC
            evrp/device/client/remoteinputlistener.cpp)
target_include_directories(device_remote_inputlistener
                           PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(device_remote_inputlistener PUBLIC evrp_device_proto
                                                         tofromproto)
set_target_properties(
  device_remote_inputlistener
  PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)

add_library(device_remote_playback STATIC evrp/device/client/remoteplayback.cpp)
target_include_directories(device_remote_playback
                           PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(device_remote_playback PUBLIC evrp_device_proto tofromproto
                                                     countingsemaphore)
set_target_properties(
  device_remote_playback
  PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)

add_library(
  evrp_device_grpc STATIC
  evrp/device/server/grpcserverimpl.cpp
  evrp/device/server/grpcinputlisten.cpp
  evrp/device/server/grpcinputdeviceservice.cpp
  evrp/device/server/grpcplaybackservice.cpp)
target_include_directories(evrp_device_grpc PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(evrp_device_grpc PUBLIC evrp_device_proto tofromproto
                                              countingsemaphore)

add_executable(evrp-device evrp-device/main.cpp)
target_link_libraries(evrp-device PRIVATE evrp_device_api evrp_device_grpc
                                          localinputlistener localplayback
                                          gRPC::grpc++ gflags::gflags)

add_executable(evrp_inputlisten_test_server evrp/inputlistentest/server.cpp)
target_include_directories(evrp_inputlisten_test_server
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(
  evrp_inputlisten_test_server
  PRIVATE evrp_device_api evrp_device_grpc localinputlistener localplayback
          gRPC::grpc++ gflags::gflags)

add_executable(evrp_inputlisten_test_client evrp/inputlistentest/client.cpp)
target_include_directories(evrp_inputlisten_test_client
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(
  evrp_inputlisten_test_client
  PRIVATE evrp_device_api device_remote_inputlistener gRPC::grpc++
          gflags::gflags)

add_executable(evrp_playback_test_server evrp/playbacktest/server.cpp)
target_include_directories(evrp_playback_test_server
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(
  evrp_playback_test_server
  PRIVATE evrp_device_api evrp_device_grpc localplayback gRPC::grpc++
          gflags::gflags)

add_executable(evrp_playback_test_client evrp/playbacktest/client.cpp)
target_include_directories(evrp_playback_test_client
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(
  evrp_playback_test_client
  PRIVATE evrp_device_api device_remote_playback gRPC::grpc++ gflags::gflags)
