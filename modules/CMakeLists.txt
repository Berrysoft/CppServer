project(Modules CXX)

add_subdirectory(shared)

add_subdirectory(file)
add_subdirectory(cpu)
add_subdirectory(disk)
add_subdirectory(markdown)
add_subdirectory(version)

configure_file(modules ${CMAKE_BINARY_DIR}/modules.txt COPYONLY)
