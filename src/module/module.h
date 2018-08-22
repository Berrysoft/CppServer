//模块类，用于动态加载模块。
#pragma once
#include <string>
#include <memory>
#include "response.h"

class module
{
private:
    void* handle;
public:
    module();
    ~module();

    void open(std::string name);
    std::unique_ptr<response> get_response(std::string command);
};
