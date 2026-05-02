cmake_minimum_required(VERSION 3.14)
project(evrp LANGUAGES C CXX)

set(EVRP_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
set(EVRP_LIBRARY_DIR "${EVRP_ROOT}/library" CACHE PATH "Third-party install prefix for this repo")

list(APPEND CMAKE_PREFIX_PATH "${EVRP_LIBRARY_DIR}")
foreach(_evrp_pkg IN ITEMS protobuf grpc gflags fmt spdlog log)
  list(APPEND CMAKE_PREFIX_PATH "${EVRP_LIBRARY_DIR}/${_evrp_pkg}")
endforeach()

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
      "Lua not found under '${EVRP_LIBRARY_DIR}/lua'. Run: ./scripts/install-third-party-to-library.sh")
endif()
add_library(lua STATIC IMPORTED GLOBAL)
set_target_properties(lua PROPERTIES IMPORTED_LOCATION "${EVRP_LUA_LIB}")
target_include_directories(lua INTERFACE "${EVRP_LUA_INCLUDE_DIR}")
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_libraries(lua INTERFACE m)
endif()

find_package(GTest CONFIG REQUIRED PATHS "${EVRP_LIBRARY_DIR}/googletest"
             NO_DEFAULT_PATH)

set(GFLAGS_USE_TARGET_NAMESPACE ON)
find_package(gflags CONFIG REQUIRED PATHS "${EVRP_LIBRARY_DIR}/gflags"
             NO_DEFAULT_PATH)
find_package(log CONFIG REQUIRED PATHS "${EVRP_LIBRARY_DIR}/log"
             NO_DEFAULT_PATH)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)
find_path(
  EVRP_ASIO_INCLUDE_DIR
  NAMES asio.hpp
  PATHS "${EVRP_LIBRARY_DIR}/asio/include"
  NO_DEFAULT_PATH)
find_library(
  EVRP_ASIO_STATIC_LIB
  NAMES asio_separate libasio_separate
  PATHS "${EVRP_LIBRARY_DIR}/asio/lib" "${EVRP_LIBRARY_DIR}/asio/lib64"
  NO_DEFAULT_PATH)
if(NOT EVRP_ASIO_INCLUDE_DIR OR NOT EVRP_ASIO_STATIC_LIB)
  message(
    FATAL_ERROR
      "Asio not found under '${EVRP_LIBRARY_DIR}/asio'. Run: ./scripts/install-third-party-to-library.sh")
endif()
add_library(asio_separate STATIC IMPORTED GLOBAL)
set_target_properties(
  asio_separate
  PROPERTIES IMPORTED_LOCATION "${EVRP_ASIO_STATIC_LIB}"
             INTERFACE_INCLUDE_DIRECTORIES "${EVRP_ASIO_INCLUDE_DIR}"
             INTERFACE_COMPILE_DEFINITIONS
             "ASIO_STANDALONE;ASIO_SEPARATE_COMPILATION"
             INTERFACE_LINK_LIBRARIES Threads::Threads)

add_library(standalone_asio INTERFACE)
add_library(asio::asio ALIAS standalone_asio)
target_link_libraries(standalone_asio INTERFACE asio_separate)

# gRPC before evrp_device / evrp_sdk (sdk_fmt); do not find_package(Protobuf) first.
find_package(gRPC CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED PATHS "${EVRP_LIBRARY_DIR}/fmt"
             NO_DEFAULT_PATH)

enable_testing()

set(EVRP_SDK_DIR "${EVRP_ROOT}/evrp/sdk")
set(EVRP_CLIENT_DIR "${EVRP_ROOT}/evrp/client")

option(EVRP_USE_X11_CURSOR "Enable X11 cursor position backend" ON)
set(EVRP_SDK_CURSOR_SRCS evrp/sdk/cursor/impl/cursorpos.cpp)
if(EVRP_USE_X11_CURSOR)
  find_package(X11 REQUIRED)
  list(APPEND EVRP_SDK_CURSOR_SRCS evrp/sdk/cursor/impl/cursorposx11.cpp)
endif()

add_library(
  evrp_sdk STATIC
  evrp/sdk/listenaddress.cpp
  evrp/sdk/setting/memorysetting.cpp
  evrp/sdk/setting/overlaysetting.cpp
  evrp/sdk/evdev.cpp
  evrp/sdk/filesystem/impl/enhancedfilesystem.cpp
  evrp/sdk/filesystem/impl/filesystem.cpp
  evrp/sdk/filesystem/impl/posixfilesystem.cpp
  ${EVRP_SDK_CURSOR_SRCS}
  evrp/sdk/playbackeventcollector.cpp
  evrp/sdk/inputeventwriter.cpp
  evrp/sdk/keyboard/keyboardeventwriter.cpp
  evrp/sdk/mouse/mouseeventwriter.cpp
  evrp/sdk/eventformat.cpp
  evrp/sdk/inputdevice.cpp
  evrp/sdk/keyboard/keyboarddevice.cpp
  evrp/sdk/touchdevice.cpp
  evrp/sdk/logger.cpp
  evrp/sdk/luaeventcomposer/luabindings.cpp
  evrp/sdk/luaeventcomposer/luaeventcomposer.cpp
  evrp/sdk/countingsemaphore.cpp
  evrp/sdk/fmt/impl/grpcstatuscode.cpp)
target_include_directories(evrp_sdk PUBLIC "${EVRP_ROOT}" ${EVRP_SDK_DIR})
if(EVRP_USE_X11_CURSOR)
  target_compile_definitions(evrp_sdk PUBLIC EVRP_USE_X11_CURSOR)
  target_link_libraries(evrp_sdk PUBLIC X11::X11)
endif()
target_link_libraries(evrp_sdk PUBLIC log::log lua gRPC::grpc++ fmt::fmt)

set(PROTO_ROOT "${EVRP_ROOT}/proto")
set(GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(DEVICE_V1_TYPES_PROTOS
  "${PROTO_ROOT}/evrp/device/v1/types/common.proto"
  "${PROTO_ROOT}/evrp/device/v1/types/inputlisten.proto"
  "${PROTO_ROOT}/evrp/device/v1/types/playback.proto"
  "${PROTO_ROOT}/evrp/device/v1/types/service.proto")
set(DEVICE_V1_TYPES_GEN
  "${GEN_DIR}/evrp/device/v1/types/common.pb.cc"
  "${GEN_DIR}/evrp/device/v1/types/common.pb.h"
  "${GEN_DIR}/evrp/device/v1/types/inputlisten.pb.cc"
  "${GEN_DIR}/evrp/device/v1/types/inputlisten.pb.h"
  "${GEN_DIR}/evrp/device/v1/types/playback.pb.cc"
  "${GEN_DIR}/evrp/device/v1/types/playback.pb.h"
  "${GEN_DIR}/evrp/device/v1/types/service.pb.cc"
  "${GEN_DIR}/evrp/device/v1/types/service.pb.h")
set(DEVICE_V1_TYPES_PB_CC
  "${GEN_DIR}/evrp/device/v1/types/common.pb.cc"
  "${GEN_DIR}/evrp/device/v1/types/inputlisten.pb.cc"
  "${GEN_DIR}/evrp/device/v1/types/playback.pb.cc"
  "${GEN_DIR}/evrp/device/v1/types/service.pb.cc")
set(DEVICE_INPUTLISTEN_PROTO "${PROTO_ROOT}/evrp/device/v1/service/inputlisten.proto")
set(DEVICE_PLAYBACK_PROTO "${PROTO_ROOT}/evrp/device/v1/service/playback.proto")
set(DEVICE_SERVICE_PROTO "${PROTO_ROOT}/evrp/device/v1/service/service.proto")
set(SDK_SESSION_TYPES_PROTO "${PROTO_ROOT}/evrp/sdk/v1/types/session.proto")
set(SDK_SESSION_SERVICES_PROTO "${PROTO_ROOT}/evrp/sdk/v1/services/session.proto")

find_program(GRPC_CPP_PLUGIN_PROGRAM NAMES grpc_cpp_plugin REQUIRED
             HINTS "${EVRP_LIBRARY_DIR}/grpc/bin" "${EVRP_LIBRARY_DIR}/bin")
find_program(PROTOC_PROGRAM NAMES protoc REQUIRED
             HINTS "${EVRP_LIBRARY_DIR}/protobuf/bin" "${EVRP_LIBRARY_DIR}/bin")

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
set(SDK_SESSION_TYPES_PB_CC "${GEN_DIR}/evrp/sdk/v1/types/session.pb.cc")
set(SDK_SESSION_TYPES_PB_H "${GEN_DIR}/evrp/sdk/v1/types/session.pb.h")
set(SDK_SESSION_SERVICES_PB_CC "${GEN_DIR}/evrp/sdk/v1/services/session.pb.cc")
set(SDK_SESSION_SERVICES_PB_H "${GEN_DIR}/evrp/sdk/v1/services/session.pb.h")
set(SDK_SESSION_SERVICES_GRPC_CC "${GEN_DIR}/evrp/sdk/v1/services/session.grpc.pb.cc")
set(SDK_SESSION_SERVICES_GRPC_H "${GEN_DIR}/evrp/sdk/v1/services/session.grpc.pb.h")

set(EVRP_DEVICE_PROTO_SOURCES
  ${DEVICE_V1_TYPES_PB_CC} ${DEVICE_INPUTLISTEN_PB_CC} ${DEVICE_INPUTLISTEN_GRPC_CC}
  ${DEVICE_PLAYBACK_PB_CC} ${DEVICE_PLAYBACK_GRPC_CC} ${DEVICE_SERVICE_PB_CC}
  ${DEVICE_SERVICE_GRPC_CC} ${SDK_SESSION_TYPES_PB_CC}
  ${SDK_SESSION_SERVICES_PB_CC} ${SDK_SESSION_SERVICES_GRPC_CC})

add_custom_command(
  OUTPUT ${DEVICE_V1_TYPES_GEN} ${DEVICE_INPUTLISTEN_PB_CC} ${DEVICE_INPUTLISTEN_PB_H}
         ${DEVICE_INPUTLISTEN_GRPC_CC} ${DEVICE_INPUTLISTEN_GRPC_H}
         ${DEVICE_PLAYBACK_PB_CC} ${DEVICE_PLAYBACK_PB_H}
         ${DEVICE_PLAYBACK_GRPC_CC} ${DEVICE_PLAYBACK_GRPC_H}
         ${DEVICE_SERVICE_PB_CC} ${DEVICE_SERVICE_PB_H}
         ${DEVICE_SERVICE_GRPC_CC} ${DEVICE_SERVICE_GRPC_H}
         ${SDK_SESSION_TYPES_PB_CC} ${SDK_SESSION_TYPES_PB_H}
         ${SDK_SESSION_SERVICES_PB_CC} ${SDK_SESSION_SERVICES_PB_H}
         ${SDK_SESSION_SERVICES_GRPC_CC} ${SDK_SESSION_SERVICES_GRPC_H}
  COMMAND ${CMAKE_COMMAND} -E make_directory "${GEN_DIR}/evrp/device/v1/service"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${GEN_DIR}/evrp/device/v1/types"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${GEN_DIR}/evrp/sdk/v1/types"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${GEN_DIR}/evrp/sdk/v1/services"
  COMMAND
    ${PROTOC_PROGRAM} --cpp_out=${GEN_DIR} --grpc_out=${GEN_DIR}
    --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN_PROGRAM} -I${PROTO_ROOT}
    ${DEVICE_V1_TYPES_PROTOS} ${DEVICE_INPUTLISTEN_PROTO} ${DEVICE_PLAYBACK_PROTO}
    ${DEVICE_SERVICE_PROTO} ${SDK_SESSION_TYPES_PROTO} ${SDK_SESSION_SERVICES_PROTO}
  DEPENDS ${DEVICE_V1_TYPES_PROTOS} ${DEVICE_INPUTLISTEN_PROTO}
          ${DEVICE_PLAYBACK_PROTO} ${DEVICE_SERVICE_PROTO} ${SDK_SESSION_TYPES_PROTO}
          ${SDK_SESSION_SERVICES_PROTO}
  COMMENT "Generating protobuf/gRPC (device v1 + session)"
  VERBATIM)

add_library(
  evrp_device STATIC
  ${EVRP_DEVICE_PROTO_SOURCES}
  evrp/sdk/sessionclient.cpp
  evrp/sdk/sessionregistry.cpp
  evrp/sdk/sessioncheck.cpp
  evrp/sdk/iocontextpostedbase.cpp
  evrp/sdk/syncdispatchqueue.cpp
  evrp/device/internal/tofromproto.cpp
  evrp/device/impl/server/localinputlistener.cpp
  evrp/device/impl/server/posted/postedinputlistener.cpp
  evrp/device/impl/server/posted/postedcursorposition.cpp
  evrp/device/impl/server/posted/postedinputdevicekindsprovider.cpp
  evrp/device/impl/server/posted/postedplayback.cpp
  evrp/device/impl/server/deviceruntime.cpp
  evrp/device/impl/server/localplayback.cpp
  evrp/device/impl/server/localcursorposition.cpp
  evrp/device/impl/server/localinputdevicekindsprovider.cpp
  evrp/device/impl/server/server.cpp
  evrp/device/internal/discovery/impl/discoveryresponder.cpp
  evrp/device/impl/server/grpc/grpcsessionservice.cpp
  evrp/device/impl/server/grpc/grpcinputlisten.cpp
  evrp/device/impl/server/grpc/grpcinputdeviceservice.cpp
  evrp/device/impl/server/grpc/grpcplaybackservice.cpp
  evrp/device/impl/client/remoteinputlistener.cpp
  evrp/device/impl/client/remoteplayback.cpp
  evrp/device/impl/client/client.cpp
  evrp/device/internal/discovery/impl/devicediscovery.cpp
  evrp/device/impl/client/remoteinputdeviceclient.cpp)
target_include_directories(
  evrp_device PUBLIC "${EVRP_ROOT}" ${EVRP_SDK_DIR} "${GEN_DIR}")
target_compile_features(evrp_device PUBLIC cxx_std_20)
target_link_libraries(
  evrp_device PUBLIC evrp_sdk protobuf::libprotobuf gRPC::grpc++ asio::asio
  PRIVATE gflags::gflags)

add_library(
  evrp_client STATIC
  evrp/client/argparser.cpp
  evrp/client/record.cpp
  evrp/client/playback.cpp
  evrp/client/runner.cpp)
target_include_directories(evrp_client PUBLIC "${EVRP_ROOT}" ${EVRP_CLIENT_DIR}
                                              ${EVRP_SDK_DIR})
target_link_libraries(evrp_client PUBLIC evrp_device evrp_sdk gflags::gflags)

add_executable(evrp evrp-client/main.cpp)
target_link_libraries(evrp PRIVATE evrp_client)

add_executable(evrp_tests tests/inputdevice_test.cpp tests/argparser_test.cpp
                                  tests/listenaddress_test.cpp
                                  tests/overlaysetting_test.cpp)
target_link_libraries(evrp_tests PRIVATE evrp_client evrp_sdk GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(evrp_tests)

add_executable(evrp-device evrp-device/main.cpp)
target_link_libraries(evrp-device PRIVATE evrp_device gRPC::grpc++ gflags::gflags)

add_executable(
  evrp_device_integration_test
  evrp-device/integrationtest/main.cpp
  evrp-device/integrationtest/harness.cpp
  evrp-device/integrationtest/case_connect.cpp
  evrp-device/integrationtest/case_inputlisten.cpp
  evrp-device/integrationtest/case_playback.cpp
  evrp-device/integrationtest/case_discovery.cpp)
target_include_directories(evrp_device_integration_test PRIVATE "${EVRP_ROOT}")
target_link_libraries(
  evrp_device_integration_test PRIVATE evrp_device gflags::gflags GTest::gtest)
add_dependencies(evrp_device_integration_test evrp-device)

gtest_discover_tests(
  evrp_device_integration_test
  DISCOVERY_MODE PRE_TEST
  EXTRA_ARGS --device_binary=$<TARGET_FILE:evrp-device>
             --listen_require_valid_event_per_kind=false)
