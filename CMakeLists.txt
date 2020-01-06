cmake_minimum_required(VERSION 3.7)
project(CppServer CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(sf CONFIG REQUIRED)
find_package(cpplinq CONFIG REQUIRED)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_subdirectory(html_writer)
add_subdirectory(read_modules)

add_subdirectory(modules)

add_subdirectory(server)

configure_file(README.md ${CMAKE_BINARY_DIR}/README.md COPYONLY)
