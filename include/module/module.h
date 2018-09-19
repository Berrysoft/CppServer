//模块类，用于动态加载模块。
#pragma once
#include <http/http_request.h>
#include <memory>
#include <module/response.h>
#include <string>

class module
{
private:
    void* handle;

public:
    module();
    ~module();

    bool open(std::string name);
    std::unique_ptr<response> get_response(const http_request& request);
    void close();
};
