cmake_minimum_required(VERSION 3.14)
project(CppServer CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CheckIPOSupported)
check_ipo_supported(RESULT ENABLE_IPO OUTPUT output)

if(ENABLE_IPO)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
    message(STATUS "LTO is enabled for Release")
endif()

find_package(sf CONFIG REQUIRED)
find_package(cpplinq CONFIG REQUIRED)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

add_subdirectory(html_writer)
add_subdirectory(read_modules)

add_subdirectory(modules)

add_subdirectory(host)

configure_file(README.md ${CMAKE_BINARY_DIR}/README.md COPYONLY)
