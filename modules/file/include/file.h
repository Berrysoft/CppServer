//读取并输出文件内容，或者动态生成主页。
#pragma once
#include <map>
#include <module/response.h>
#include <string>

class file_response
{
private:
    std::string filename;
    std::map<std::string, std::string> requires;
    bool israw;
    bool file_exists;

public:
    file_response(init_response_arg* request);
    ~file_response() {}
    int status();
    const char* type();
    ssize_t send(int fd);
};
