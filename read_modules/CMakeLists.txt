project(ReadModules CXX)

aux_source_directory(src READ_MODULES_SOURCE)
add_library(read_modules SHARED ${READ_MODULES_SOURCE})
target_include_directories(read_modules PUBLIC include)
