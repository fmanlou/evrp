add_executable(evrp-server main.cpp)
target_link_libraries(evrp-server PRIVATE evrp_server evrp_server_argparser)
set_target_properties(evrp-server PROPERTIES RUNTIME_OUTPUT_DIRECTORY
                                       "${CMAKE_BINARY_DIR}/bin")
