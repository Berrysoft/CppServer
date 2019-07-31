cmake_minimum_required(VERSION 3.1)
project(CppServer)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
include_directories(include)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -flto")

aux_source_directory(${PROJECT_SOURCE_DIR}/src/html HTML_SOURCE)
add_library(html_writer SHARED ${HTML_SOURCE})

aux_source_directory(${PROJECT_SOURCE_DIR}/src/read_modules READ_MODULES_SOURCE)
add_library(read_modules SHARED ${READ_MODULES_SOURCE})

configure_file(${PROJECT_SOURCE_DIR}/src/html/style.css style.css COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/src/module/modules modules COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/README.md README.md COPYONLY)

aux_source_directory(${PROJECT_SOURCE_DIR}/src/module/file/ FILE_SOURCE)
add_library(file SHARED ${FILE_SOURCE})
target_link_libraries(file html_writer read_modules)

aux_source_directory(${PROJECT_SOURCE_DIR}/src/module/cpu/ CPU_SOURCE)
add_library(cpu SHARED ${CPU_SOURCE})
target_link_libraries(cpu html_writer)

aux_source_directory(${PROJECT_SOURCE_DIR}/src/module/disk/ DISK_SOURCE)
add_library(disk SHARED ${DISK_SOURCE})
target_link_libraries(disk html_writer)

aux_source_directory(${PROJECT_SOURCE_DIR}/src/module/markdown/ MD_SOURCE)
add_library(markdown SHARED ${MD_SOURCE})
target_link_libraries(markdown html_writer)

aux_source_directory(${PROJECT_SOURCE_DIR}/src/module/version/ V_SOURCE)
add_library(version SHARED ${V_SOURCE})
target_link_libraries(version html_writer)

aux_source_directory(${PROJECT_SOURCE_DIR}/src SOURCE)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/http HTTP_SOURCE)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/module MODULE_SOURCE)
add_executable(server ${SOURCE} ${HTTP_SOURCE} ${MODULE_SOURCE})
target_link_libraries(server read_modules pthread dl)
