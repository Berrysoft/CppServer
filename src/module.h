//模块类，用于动态加载模块。
#pragma once
#include "mdl/response.h"

class module
{
private:
    char filename[64];
    void* handle;
public:
    module(){}
    module(const char *name);

    bool open();
    response *get_response(const char *command);
    void close();
};
