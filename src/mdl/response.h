//在动态模块中用来向文件描述符写HTML的抽象类。
#pragma once
#include <unistd.h>

class response
{
public:
    virtual ~response(){}
    virtual ssize_t send(int fd) = 0;
};

extern "C" void *get_instance_response(const char *command);
