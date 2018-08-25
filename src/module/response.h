//在动态模块中用来向文件描述符写HTML的抽象类。
#pragma once
#include <string>
#include <unistd.h>

class response
{
public:
    virtual ~response() {}
    virtual bool supports(std::string version) { return version != "HTTP/1.0"; }
    virtual int status() { return 200; }
    virtual long long length() { return -1; }
    virtual std::string type() { return "text/html"; }
    virtual ssize_t send(int fd) = 0;
};

//这是为动态库提供的接口，因此采用C语言的指针与参数
extern "C" void *get_instance_response(const char *command);
