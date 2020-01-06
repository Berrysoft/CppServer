project(VersionModule CXX)

aux_source_directory(src VERSION_SOURCE)
add_library(version SHARED ${VERSION_SOURCE})
target_include_directories(version PRIVATE include)
target_link_libraries(version modules_shared html_writer)
