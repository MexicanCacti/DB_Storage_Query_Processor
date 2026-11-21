cmake_minimum_required(VERSION 3.10)
project(MINI_DB_TESTS)

add_executable(query_demo
    query_demo.cpp
)

target_link_libraries(query_demo
    PRIVATE
        DB_STORAGE_LIBRARY
        QUERY_PROCESSOR_LIBRARY
)

set_target_properties(
    query_demo
      PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)