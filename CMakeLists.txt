cmake_minimum_required(VERSION 3.1)

project(c0-parser)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

enable_testing()

add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(example)