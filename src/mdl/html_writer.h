//用来向一个文件描述符写HTML文档，
//需要用到程序运行目录下的`style.css`样式表。
//由于文档是动态生成的，所以采用Chunked编码方式传输。
#pragma once
#include <unistd.h>
#include <string>
#include <vector>

ssize_t send_with_chunk(int fd, const void *buffer, size_t length, int flag);

class html_writer
{
  private:
    int fd;

    ssize_t write_spe(std::string spe, std::string text);

  public:
    html_writer(int fd) : fd(fd) {}

    ssize_t write_head(std::string title);
    ssize_t write_h1(std::string title);
    ssize_t write_p(std::string text);
    ssize_t write_h2(std::string title);
    ssize_t write_h3(std::string title);
    ssize_t write_ul(std::vector<std::string> texts);

    ssize_t write_table_start(std::vector<std::string> texts);
    ssize_t write_tr(std::vector<std::string> texts);
    ssize_t write_table_end();

    ssize_t write_pre_code_start();
    ssize_t write_pre_code_end();
    ssize_t write_code_start();
    ssize_t write_code_end();

    ssize_t write_xmp_start();
    ssize_t write_xmp_end();

    ssize_t write_end();
};
