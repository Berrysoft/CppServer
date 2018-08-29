//读取并输出文件内容，或者动态生成主页。
#pragma once
#include "../response.h"
#include <string>

class file_response : public response
{
private:
    std::string filename;

public:
    file_response(const http_request& request, std::string filename);
    ~file_response() {}

    ssize_t send(int fd);
};
