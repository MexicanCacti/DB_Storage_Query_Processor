cmake_minimum_required(VERSION 3.10)
project(MINI_DB LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_library(${PROJECT_NAME})

# Include all .cpp files
target_sources(${PROJECT_NAME}
    PRIVATE
    main.cpp
        
)

# Include all Directories
target_include_directories(${PROJECT_NAME}
    PRIVATE 

    # Include all directories that contain the .hpp files
    PUBLIC
        # where top-level project will look for the library's public headers

        #$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/engine/basic_types>




        # where external projects will look for the library's public headers
        #$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

add_subdirectory(storage_engine)
add_subdirectory(query_processor)
add_subdirectory(query_processor/google_tests)

add_executable(main_test
    main.cpp

)

target_link_libraries(main_test
    PRIVATE 
        DB_STORAGE_LIBRARY
)

set_target_properties(
    main_test
      PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)