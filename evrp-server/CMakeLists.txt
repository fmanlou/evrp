add_executable(evrp-server main.cpp)
target_link_libraries(evrp-server PRIVATE evrp_server)
set_target_properties(evrp-server PROPERTIES RUNTIME_OUTPUT_DIRECTORY
                                       "${CMAKE_CURRENT_BINARY_DIR}")
