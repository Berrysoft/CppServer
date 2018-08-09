//模块类，用于动态加载模块。
#pragma once
#include <string>
#include <memory>
#include <functional>
#include "mdl/response.h"

class module
{
private:
    std::string filename;
    void* handle;
public:
    module(){}
    module(std::string name);

    bool open();
    std::unique_ptr<response> get_response(std::string command);
    void close();
};
