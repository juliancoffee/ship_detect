cmake_minimum_required(VERSION 3.10)

# Project name
project(ShipDetect)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add external dependencies
add_subdirectory(external/fmt)
find_package(OpenCV REQUIRED)

# Add your source files
add_executable(main main.cpp)
include_directories(lib)

# Register external dependencies
target_link_libraries(main fmt)
target_link_libraries(main ${OpenCV_LIBS})
