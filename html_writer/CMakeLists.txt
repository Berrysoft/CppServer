project(HtmlWriter CXX)

aux_source_directory(src HTML_WRITER_SOURCE)
add_library(html_writer SHARED ${HTML_WRITER_SOURCE})
target_include_directories(html_writer PUBLIC include)

configure_file(src/style.css ${CMAKE_BINARY_DIR}/style.css COPYONLY)
