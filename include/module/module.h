//模块类，用于动态加载模块。
#pragma once
#include <functional>
#include <http/http_request.h>
#include <memory>
#include <module/response.h>
#include <string>

class module
{
private:
    void* handle;

public:
    using response_ptr = std::unique_ptr<response, std::function<void(void*)>>;

    module();
    ~module();

    bool open(std::string name);
    response_ptr get_response(const http_request& request);
    void close();
};
