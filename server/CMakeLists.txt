project(Server CXX)

aux_source_directory(${PROJECT_SOURCE_DIR}/src SOURCE)
add_executable(cpp_server ${SOURCE})
target_include_directories(cpp_server PRIVATE include)
target_link_libraries(cpp_server modules_shared html_writer read_modules pthread dl)
