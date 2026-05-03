add_executable(evrp-device main.cpp)
target_link_libraries(evrp-device PRIVATE evrp_device gRPC::grpc++ gflags::gflags)
set_target_properties(evrp-device PROPERTIES RUNTIME_OUTPUT_DIRECTORY
                                            "${CMAKE_BINARY_DIR}")

add_executable(
  evrp_device_integration_test
  integrationtest/main.cpp
  integrationtest/harness.cpp
  integrationtest/case_connect.cpp
  integrationtest/case_inputlisten.cpp
  integrationtest/case_playback.cpp
  integrationtest/case_discovery.cpp)
target_include_directories(evrp_device_integration_test
                           PRIVATE "${CMAKE_SOURCE_DIR}")
target_link_libraries(evrp_device_integration_test PRIVATE evrp_device gflags::gflags
                                                            GTest::gtest)
add_dependencies(evrp_device_integration_test evrp-device)

set_target_properties(evrp_device_integration_test
                      PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

gtest_discover_tests(
  evrp_device_integration_test
  DISCOVERY_MODE PRE_TEST
  EXTRA_ARGS --device_binary=$<TARGET_FILE:evrp-device>
             --listen_require_valid_event_per_kind=false)
