project(Server CXX)

aux_source_directory(${PROJECT_SOURCE_DIR}/src SOURCE)
add_executable(server ${SOURCE})
target_include_directories(server PRIVATE include)
target_link_libraries(server modules_shared html_writer read_modules pthread dl)
