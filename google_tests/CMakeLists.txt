cmake_minimum_required(VERSION 3.10)
project(MINI_DB_GOOGLE_TESTS)

# GoogleTest requires at least C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(all_expression_tests
    expression_tests/addition_expression_tests.cpp
    expression_tests/subtraction_expression_tests.cpp
    expression_tests/equal_expression_tests.cpp
    expression_tests/not_equal_expression_tests.cpp
    expression_tests/greater_than_expression_tests.cpp
    expression_tests/greater_than_equal_expression_tests.cpp
    expression_tests/less_than_expression_tests.cpp
    expression_tests/less_than_equal_expression_tests.cpp
    expression_tests/and_expression_tests.cpp
    expression_tests/or_expression_tests.cpp
    expression_tests/not_expression_tests.cpp
)

add_executable(all_executor_tests
  executor_tests/values_executor_tests.cpp
)

target_link_libraries(all_expression_tests
    PRIVATE
        DB_STORAGE_LIBRARY
        QUERY_PROCESSOR_LIBRARY
        GTest::gtest_main
)

target_link_libraries(all_executor_tests
    PRIVATE
        DB_STORAGE_LIBRARY
        QUERY_PROCESSOR_LIBRARY
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(all_expression_tests)

set_target_properties(
    all_expression_tests
      PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Expression_Tests"
)

set_target_properties(
    all_executor_tests
      PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Executor_Tests"
)