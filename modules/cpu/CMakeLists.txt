project(CPUModule CXX)

aux_source_directory(src CPU_SOURCE)
add_library(cpu SHARED ${CPU_SOURCE})
target_include_directories(cpu PRIVATE include)
target_link_libraries(cpu modules_shared html_writer)
