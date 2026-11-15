cmake_minimum_required(VERSION 3.10)
project(MINI_DB LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

enable_testing()

add_subdirectory(storage_engine)
add_subdirectory(query_processor)
add_subdirectory(query_processor/google_tests)

add_executable(main_test
    main.cpp
)

target_link_libraries(main_test
    PRIVATE 
        DB_STORAGE_LIBRARY
        QUERY_PROCESSOR_LIBRARY
)

set_target_properties(
    main_test
      PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)