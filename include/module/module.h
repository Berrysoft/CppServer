//模块类，用于动态加载模块。
#pragma once
#include <functional>
#include <http/http_request.h>
#include <memory>
#include <string>

class module
{
private:
    void* handle;

public:
    module();
    ~module();

    bool open(std::string name);
    bool init(const http_request& request);
    std::int32_t status();
    std::int64_t length();
    const char* type();
    ssize_t send(int fd);
    bool destory();
    void close();
};
