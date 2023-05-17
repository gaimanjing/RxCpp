cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

# define some folders

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(example)

include(CTest)
add_subdirectory(test)
