//一个简易Markdown引擎.
#pragma once
#include <module/response.h>
#include <string>

class markdown_response
{
private:
    std::string filename;
    bool file_exists;

public:
    markdown_response(init_response_arg* arg);
    ~markdown_response() {}

    ssize_t send(int fd);
};
