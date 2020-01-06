project(FileModule CXX)

aux_source_directory(src FILE_SOURCE)
add_library(file SHARED ${FILE_SOURCE})
target_include_directories(file PRIVATE include)
target_link_libraries(file modules_shared html_writer read_modules)
