project(MarkdownModule CXX)

aux_source_directory(src MARKDOWN_SOURCE)
add_library(markdown SHARED ${MARKDOWN_SOURCE})
target_include_directories(markdown PRIVATE include)
target_link_libraries(markdown modules_shared html_writer md4c-html)
