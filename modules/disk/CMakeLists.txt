project(DiskModule CXX)

aux_source_directory(src DISK_SOURCE)
add_library(disk SHARED ${DISK_SOURCE})
target_link_libraries(disk modules_shared html_writer)
