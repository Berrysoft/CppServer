//读取并输出文件内容，或者动态生成主页。
#pragma once
#include "../response.h"
#include <string>
#include <map>

class file_response : public response
{
private:
    std::string filename;
    std::map<std::string, std::string> requires;
    bool israw;
    bool file_exists;

public:
    file_response(const http_request& request, std::string filename);
    ~file_response() {}
    int status();
    std::string type();
    ssize_t send(int fd);
};
