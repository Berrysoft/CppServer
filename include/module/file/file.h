//读取并输出文件内容，或者动态生成主页。
#pragma once
#include <map>
#include <module/response.h>
#include <string>

class file_response : public response
{
private:
    std::string filename;
    std::map<std::string, std::string> requires;
    bool israw;
    bool file_exists;

public:
    file_response(const http_request& request, std::string filename);
    ~file_response() override {}
    int status() override;
    std::string type() override;
    ssize_t send(int fd) override;
};
