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

enable_testing()

set(EVRP_SDK_DIR "${EVRP_ROOT}/evrp/sdk")
set(EVRP_CLIENT_DIR "${EVRP_ROOT}/evrp/client")

# --- client / sdk static libs ---
add_library(setting STATIC evrp/sdk/setting/memorysetting.cpp)
target_include_directories(setting PUBLIC "${EVRP_ROOT}" ${EVRP_SDK_DIR})

add_library(evdev STATIC evrp/sdk/evdev.cpp)
target_include_directories(evdev PUBLIC "${EVRP_ROOT}" ${EVRP_SDK_DIR})

add_library(filesystem STATIC
  evrp/sdk/filesystem/impl/enhancedfilesystem.cpp
  evrp/sdk/filesystem/impl/filesystem.cpp
  evrp/sdk/filesystem/impl/posixfilesystem.cpp)
target_include_directories(filesystem PUBLIC "${EVRP_ROOT}" ${EVRP_SDK_DIR})
target_link_libraries(evdev PRIVATE filesystem)

option(EVRP_USE_X11_CURSOR "Enable X11 cursor position backend" ON)
set(cursorpos_sources evrp/sdk/cursor/impl/cursorpos.cpp)
if(EVRP_USE_X11_CURSOR)
  find_package(X11 REQUIRED)
  list(APPEND cursorpos_sources evrp/sdk/cursor/impl/cursorposx11.cpp)
endif()
add_library(cursorpos STATIC ${cursorpos_sources})
target_include_directories(cursorpos PUBLIC "${EVRP_ROOT}" ${EVRP_SDK_DIR})
if(EVRP_USE_X11_CURSOR)
  target_compile_definitions(cursorpos PUBLIC EVRP_USE_X11_CURSOR)
  target_link_libraries(cursorpos PUBLIC X11::X11)
endif()

add_library(argparser STATIC evrp/client/argparser.cpp)
target_include_directories(argparser PUBLIC "${EVRP_ROOT}" ${EVRP_CLIENT_DIR}
                                            ${EVRP_SDK_DIR})
target_link_libraries(argparser PUBLIC setting PRIVATE logger gflags::gflags)

add_library(playbackeventcollector STATIC evrp/sdk/playbackeventcollector.cpp)
target_include_directories(playbackeventcollector PUBLIC "${EVRP_ROOT}"
                                                         ${EVRP_SDK_DIR})
target_link_libraries(playbackeventcollector PRIVATE logger)

add_library(
  inputeventwriter STATIC
  evrp/sdk/inputeventwriter.cpp
  evrp/sdk/keyboard/keyboardeventwriter.cpp
  evrp/sdk/mouse/mouseeventwriter.cpp)
target_include_directories(inputeventwriter PUBLIC "${EVRP_ROOT}" ${EVRP_SDK_DIR})
target_link_libraries(
  inputeventwriter PRIVATE evdev eventformat filesystem inputdevice logger
                           cursorpos)

add_library(eventformat STATIC evrp/sdk/eventformat.cpp)
target_include_directories(eventformat PUBLIC "${EVRP_ROOT}" ${EVRP_SDK_DIR})
target_link_libraries(eventformat PRIVATE evdev inputdevice)

add_library(
  inputdevice STATIC
  evrp/sdk/inputdevice.cpp
  evrp/sdk/keyboard/keyboarddevice.cpp
  evrp/sdk/touchdevice.cpp)
target_include_directories(inputdevice PUBLIC "${EVRP_ROOT}" ${EVRP_SDK_DIR})
target_link_libraries(inputdevice PUBLIC evdev filesystem)

add_library(record STATIC evrp/client/record.cpp)
target_include_directories(record PUBLIC "${EVRP_ROOT}" ${EVRP_CLIENT_DIR}
                                         ${EVRP_SDK_DIR})
target_link_libraries(record PRIVATE evdev eventformat filesystem logger)

add_library(logger STATIC evrp/sdk/logger.cpp)
target_include_directories(logger PUBLIC "${EVRP_ROOT}" ${EVRP_SDK_DIR})
target_link_libraries(logger PUBLIC log::log)

add_library(playback STATIC evrp/client/playback.cpp)
target_include_directories(playback PUBLIC "${EVRP_ROOT}" ${EVRP_CLIENT_DIR}
                                           ${EVRP_SDK_DIR})
target_link_libraries(
  playback PRIVATE evdev filesystem logger luaeventcomposer setting)

add_library(luabindings STATIC evrp/sdk/luaeventcomposer/luabindings.cpp)
target_include_directories(luabindings PUBLIC "${EVRP_ROOT}" ${EVRP_SDK_DIR})
target_link_libraries(
  luabindings PUBLIC lua inputeventwriter playbackeventcollector filesystem
                     logger cursorpos)

add_library(luaeventcomposer STATIC
            evrp/sdk/luaeventcomposer/luaeventcomposer.cpp)
target_include_directories(luaeventcomposer PUBLIC "${EVRP_ROOT}" ${EVRP_SDK_DIR})
target_link_libraries(luaeventcomposer PRIVATE eventformat luabindings
                                               playbackeventcollector)

add_library(runner STATIC evrp/client/runner.cpp)
target_include_directories(runner PUBLIC "${EVRP_ROOT}" ${EVRP_CLIENT_DIR}
                                         ${EVRP_SDK_DIR})
target_link_libraries(
  runner PUBLIC argparser playback record filesystem logger device_client
                setting)

add_executable(evrp evrp-client/main.cpp)
target_link_libraries(evrp PRIVATE runner inputdevice evdev luabindings logger
                                   cursorpos)

add_executable(evrp_tests tests/inputdevice_test.cpp tests/argparser_test.cpp)
target_link_libraries(evrp_tests PRIVATE inputdevice argparser logger
                                         GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(evrp_tests)

# --- gRPC / device (do not find_package(Protobuf) before gRPC) ---
find_package(gRPC CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED PATHS "${EVRP_LIBRARY_DIR}/fmt"
             NO_DEFAULT_PATH)

add_library(evrp_device_cxx20 INTERFACE)
target_compile_features(evrp_device_cxx20 INTERFACE cxx_std_20)

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

add_library(evrp_device_proto STATIC ${DEVICE_V1_TYPES_PB_CC}
  ${DEVICE_INPUTLISTEN_PB_CC} ${DEVICE_INPUTLISTEN_GRPC_CC} ${DEVICE_PLAYBACK_PB_CC}
  ${DEVICE_PLAYBACK_GRPC_CC} ${DEVICE_SERVICE_PB_CC} ${DEVICE_SERVICE_GRPC_CC}
  ${SDK_SESSION_TYPES_PB_CC} ${SDK_SESSION_SERVICES_PB_CC}
  ${SDK_SESSION_SERVICES_GRPC_CC})
target_include_directories(evrp_device_proto PUBLIC ${GEN_DIR})
target_link_libraries(evrp_device_proto PUBLIC protobuf::libprotobuf gRPC::grpc++)

add_library(evrp_device_api INTERFACE)
target_include_directories(evrp_device_api INTERFACE "${EVRP_ROOT}")

add_library(countingsemaphore STATIC evrp/sdk/countingsemaphore.cpp)
target_include_directories(countingsemaphore PUBLIC "${EVRP_ROOT}")

add_library(sdk_fmt STATIC evrp/sdk/fmt/impl/grpcstatuscode.cpp)
target_include_directories(sdk_fmt PUBLIC "${EVRP_ROOT}")
target_link_libraries(sdk_fmt PUBLIC gRPC::grpc++ fmt::fmt)

add_library(sdk_session_client STATIC evrp/sdk/sessionclient.cpp)
target_include_directories(sdk_session_client PUBLIC "${EVRP_ROOT}")
target_link_libraries(
  sdk_session_client PUBLIC evrp_device_proto gRPC::grpc++
  PRIVATE evrp_device_cxx20)

add_library(sdk_session STATIC evrp/sdk/sessionregistry.cpp
                               evrp/sdk/sessioncheck.cpp)
target_include_directories(sdk_session PUBLIC "${EVRP_ROOT}")
target_link_libraries(
  sdk_session PUBLIC gRPC::grpc++ gflags::gflags PRIVATE logger evrp_device_cxx20)

add_library(tofromproto STATIC evrp/device/internal/tofromproto.cpp)
target_include_directories(tofromproto PUBLIC "${EVRP_ROOT}")
target_link_libraries(tofromproto PUBLIC evrp_device_api evrp_device_proto
                                         PRIVATE evrp_device_cxx20)

add_library(
  localinputlistener STATIC
  evrp/device/impl/server/localinputlistener.cpp
  evrp/sdk/iocontextpostedbase.cpp
  evrp/device/impl/server/posted/postedinputlistener.cpp
  evrp/device/impl/server/posted/postedcursorposition.cpp
  evrp/device/impl/server/posted/postedinputdevicekindsprovider.cpp
  evrp/device/impl/server/posted/postedplayback.cpp
  evrp/sdk/syncdispatchqueue.cpp
  evrp/device/impl/server/deviceruntime.cpp)
target_include_directories(localinputlistener PUBLIC "${EVRP_ROOT}" ${EVRP_SDK_DIR})
target_link_libraries(
  localinputlistener PUBLIC evrp_device_api asio::asio
  PRIVATE eventformat inputdevice logger evrp_device_cxx20)

add_library(localplayback STATIC evrp/device/impl/server/localplayback.cpp)
target_include_directories(localplayback PUBLIC "${EVRP_ROOT}")
target_link_libraries(
  localplayback PUBLIC evrp_device_api inputeventwriter countingsemaphore
  PRIVATE evrp_device_cxx20)

add_library(localcursorposition STATIC
            evrp/device/impl/server/localcursorposition.cpp)
target_include_directories(localcursorposition PUBLIC "${EVRP_ROOT}")
target_link_libraries(localcursorposition PUBLIC evrp_device_api cursorpos)

add_library(localinputdevicekindsprovider STATIC
            evrp/device/impl/server/localinputdevicekindsprovider.cpp)
target_include_directories(localinputdevicekindsprovider PUBLIC "${EVRP_ROOT}")
target_link_libraries(localinputdevicekindsprovider PUBLIC evrp_device_api
                      PRIVATE inputdevice logger)

add_library(device_remote_inputlistener STATIC
            evrp/device/impl/client/remoteinputlistener.cpp)
target_include_directories(device_remote_inputlistener PUBLIC "${EVRP_ROOT}")
target_link_libraries(
  device_remote_inputlistener PUBLIC evrp_device_proto tofromproto sdk_fmt
  PRIVATE logger evrp_device_cxx20)

add_library(device_remote_playback STATIC
            evrp/device/impl/client/remoteplayback.cpp)
target_include_directories(device_remote_playback PUBLIC "${EVRP_ROOT}")
target_link_libraries(
  device_remote_playback PUBLIC evrp_device_proto tofromproto countingsemaphore
  PRIVATE evrp_device_cxx20)

add_library(
  device_client STATIC
  evrp/device/impl/client/client.cpp
  evrp/device/impl/client/devicediscovery.cpp
  evrp/device/impl/client/remoteinputdeviceclient.cpp)
target_include_directories(device_client PUBLIC "${EVRP_ROOT}")
target_link_libraries(
  device_client PUBLIC evrp_device_api device_remote_inputlistener
                       device_remote_playback evrp_device_proto sdk_session_client
                       tofromproto gRPC::grpc++
  PRIVATE gflags::gflags evrp_device_cxx20)

add_library(
  evrp_device_grpc STATIC
  evrp/device/impl/server/server.cpp
  evrp/device/impl/server/discoveryresponder.cpp
  evrp/device/impl/server/grpc/grpcsessionservice.cpp
  evrp/device/impl/server/grpc/grpcinputlisten.cpp
  evrp/device/impl/server/grpc/grpcinputdeviceservice.cpp
  evrp/device/impl/server/grpc/grpcplaybackservice.cpp)
target_include_directories(evrp_device_grpc PRIVATE "${EVRP_ROOT}")
target_link_libraries(
  evrp_device_grpc PUBLIC evrp_device_proto tofromproto countingsemaphore
                           asio::asio gflags::gflags sdk_session
  PRIVATE logger)

add_executable(evrp-device evrp-device/main.cpp)
target_link_libraries(
  evrp-device PRIVATE evrp_device_api evrp_device_grpc localinputlistener
                      localplayback localcursorposition
                      localinputdevicekindsprovider gRPC::grpc++ gflags::gflags
                      logger)

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
  evrp_device_integration_test PRIVATE device_client gflags::gflags logger
                                       GTest::gtest evrp_device_cxx20)
add_dependencies(evrp_device_integration_test evrp-device)

gtest_discover_tests(
  evrp_device_integration_test
  DISCOVERY_MODE PRE_TEST
  EXTRA_ARGS --device_binary=$<TARGET_FILE:evrp-device>
             --listen_require_valid_event_per_kind=false)
