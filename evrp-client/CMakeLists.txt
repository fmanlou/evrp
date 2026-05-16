add_executable(evrp-client main.cpp runner.cpp)
target_include_directories(evrp-client PRIVATE "${CMAKE_SOURCE_DIR}")
if(UNIX AND NOT APPLE)
  target_link_libraries(
    evrp-client PRIVATE "-Wl,--start-group" evrp_server evrp_client_argparser
                        evrp_device evrp_sdk "-Wl,--end-group")
else()
  target_link_libraries(evrp-client PRIVATE evrp_server evrp_client_argparser
                                            evrp_device evrp_sdk)
endif()
set_target_properties(evrp-client PROPERTIES RUNTIME_OUTPUT_DIRECTORY
                                             "${CMAKE_BINARY_DIR}/bin")
