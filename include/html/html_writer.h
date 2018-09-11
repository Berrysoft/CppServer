//用来向一个文件描述符写HTML文档，
//需要用到程序运行目录下的`style.css`样式表。
//由于文档是动态生成的，所以采用Chunked编码方式传输。
#pragma once
#include <string>
#include <unistd.h>
#include <vector>

#define INIT_RESULT_AND_TEMP ssize_t result = 0, t
#define IF_NEGATIVE_EXIT(exp) \
    if ((t = (exp)) < 0)      \
        return t;             \
    result += t
#define RETURN_RESULT return result

ssize_t send_with_chunk(int fd, const char* buffer, size_t length, int flag);

template <size_t size>
inline ssize_t send_with_chunk(int fd, const char (&buffer)[size], int flag)
{
    return send_with_chunk(fd, buffer, size - 1, flag);
}

inline ssize_t send_with_chunk(int fd, const std::string& buffer, int flag)
{
    return send_with_chunk(fd, buffer.c_str(), buffer.length(), flag);
}

ssize_t send_chunk_end(int fd, int flag);

class html_writer
{
private:
    int fd;

    ssize_t write_spe(std::string spe, std::string text);
    ssize_t write_spe_start(std::string spe);
    ssize_t write_spe_end(std::string spe);

public:
    html_writer(int fd) : fd(fd) {}

    ssize_t write_head(std::string title);
    ssize_t write_h1(std::string title);
    ssize_t write_h2(std::string title);
    ssize_t write_h3(std::string title);
    ssize_t write_ul(std::vector<std::string> texts);

    ssize_t write_p(std::string text);
    ssize_t write_p_start();
    ssize_t write_p_end();

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
