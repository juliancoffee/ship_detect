cmake_minimum_required(VERSION 3.10)

project(ShipDetect)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "-Wall -Wextra")

# Deps
add_subdirectory(external/fmt)
find_package(OpenCV REQUIRED)

# Libs
add_library(utils INTERFACE)
target_include_directories(utils INTERFACE lib)
target_link_libraries(utils INTERFACE fmt ${OpenCV_LIBS})

# Binaries
add_executable(main main.cpp)
target_link_libraries(main utils)

add_executable(example example.cpp)
target_link_libraries(example utils)
