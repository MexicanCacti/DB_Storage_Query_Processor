### Github Link: https://github.com/MexicanCacti/DB_Storage_Query_Processor

# Development Environment

This project is implemented in C++ using CMake.

# Build requirements

C++17-compatible compiler or newer

CMake 3.14 or newer

GoogleTest for unit testing

Windows or another supported operating system

# Run Instructions 
(From Top-Level Directory [...\DB_Storage_Query_Processor] )

- cmake -S . -B build
- cmake --build build --config Debug

# Running Tests
(From Top-Level Directory [...\DB_Storage_Query_Processor] )
- [Google_Tests] <br> 

# My Development Environment Info
- Operating System: Windows
- System Architecture: AMD64
- C++ Compiler: MSVC
- Compiler Version: 19.51.36246.0
- C++ Standard: 17
- CMake Version: 4.3.3
- Build Generator: Visual Studio 18 2026

# Project Organization
- storage_engine/ - Contains the storage engine library
- google_tests/ - Unit tests written with GoogleTests for testing class functionality
- tests/ - Hand-written tests used to demonstrate correctness

## Current Implementation Notes

- Cmake will throw errors but the code will run fine! Currently working on fixing that x3
