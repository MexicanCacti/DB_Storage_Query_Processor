cmake_minimum_required(VERSION 3.10)
project(DB_STORAGE_LIBRARY LANGUAGES CXX)

add_library(${PROJECT_NAME} STATIC)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

target_sources(${PROJECT_NAME}
    PRIVATE
        engine/basic_types/basic_types.cpp
        engine/basic_types/basic_types_encode_decode.cpp
        engine/basic_types/bitmap.cpp
        engine/helpers/helpers.cpp
        engine/page/page.cpp
        engine/schema/schema.cpp
        engine/tuple/tuple.cpp
        engine/table/table_header.cpp
        engine/data_page/data_page_header.cpp
        engine/data_page/data_page.cpp
        engine/file_manager/file_manager.cpp
        engine/buffer_pool/buffer_pool.cpp
        engine/database/database.cpp
)

target_include_directories(${PROJECT_NAME}
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/engine/helpers
        ${CMAKE_CURRENT_SOURCE_DIR}/engine/page
        ${CMAKE_CURRENT_SOURCE_DIR}/engine/table
        ${CMAKE_CURRENT_SOURCE_DIR}/engine/file_manager
        ${CMAKE_CURRENT_SOURCE_DIR}/engine/buffer_pool
        ${CMAKE_CURRENT_SOURCE_DIR}/engine/database
        ${CMAKE_CURRENT_SOURCE_DIR}/engine/schema
        ${CMAKE_CURRENT_SOURCE_DIR}/engine/tuple
        ${CMAKE_CURRENT_SOURCE_DIR}/engine/basic_types
        ${CMAKE_CURRENT_SOURCE_DIR}/engine/data_page
)