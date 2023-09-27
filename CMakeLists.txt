cmake_minimum_required(VERSION 3.10)

# Project name
project(ShipDetect)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add external dependencies
add_subdirectory(external/fmt)

# Add your source files
add_executable(main main.cpp)

# Register external dependencies
target_link_libraries(main fmt)
