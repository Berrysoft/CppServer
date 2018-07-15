#pragma once
#include "response.h"

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
