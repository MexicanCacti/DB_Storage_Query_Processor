# Packages the storage engine into a library & also creates the runner & test executables 
cmake_minimum_required(VERSION 3.10)
project(QUERY_PROCESSOR_LIBRARY LANGUAGES CXX)

add_library(${PROJECT_NAME} STATIC)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

# Include all .cpp files
target_sources(${PROJECT_NAME}
    PRIVATE
        processor/expressions/column_expression.cpp
        processor/expressions/addition_expression.cpp
        processor/expressions/subtraction_expression.cpp
        processor/expressions/equal_expression.cpp
        processor/expressions/not_equal_expression.cpp
        processor/expressions/greater_than_expression.cpp
        processor/expressions/greater_than_equal_expression.cpp
        processor/expressions/less_than_expression.cpp
        processor/expressions/less_than_equal_expression.cpp
        processor/expressions/and_expression.cpp
        processor/expressions/or_expression.cpp
        processor/expressions/not_expression.cpp
        processor/executors/values_executor.cpp

)

target_include_directories(${PROJECT_NAME}
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/processor
        ${CMAKE_CURRENT_SOURCE_DIR}/processor/expressions
        ${CMAKE_CURRENT_SOURCE_DIR}/processor/executors
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        DB_STORAGE_LIBRARY
)