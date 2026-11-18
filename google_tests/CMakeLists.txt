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

add_executable( addition_expression_tests
  addition_expression_tests.cpp
)

add_executable( subtraction_expression_tests
  subtraction_expression_tests.cpp
)

add_executable(all_tests
    addition_expression_tests.cpp
    subtraction_expression_tests.cpp
)

target_link_libraries( addition_expression_tests
    PRIVATE
        DB_STORAGE_LIBRARY
        QUERY_PROCESSOR_LIBRARY
        GTest::gtest_main
)

target_link_libraries( subtraction_expression_tests
    PRIVATE
        DB_STORAGE_LIBRARY
        QUERY_PROCESSOR_LIBRARY
        GTest::gtest_main
)

target_link_libraries(all_tests
    PRIVATE
        DB_STORAGE_LIBRARY
        QUERY_PROCESSOR_LIBRARY
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(addition_expression_tests)
gtest_discover_tests(subtraction_expression_tests)
gtest_discover_tests(all_tests)

set_target_properties(
    addition_expression_tests subtraction_expression_tests all_tests
      PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)