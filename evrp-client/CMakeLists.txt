add_executable(evrp main.cpp runner.cpp)
target_link_libraries(evrp PRIVATE evrp_client)
set_target_properties(evrp PROPERTIES RUNTIME_OUTPUT_DIRECTORY
                                       "${CMAKE_BINARY_DIR}")
