cmake_minimum_required(VERSION 3.10)
project(MINI_DB LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

enable_testing()

add_subdirectory(storage_engine)
add_subdirectory(query_processor)
add_subdirectory(google_tests)
add_subdirectory(tests)