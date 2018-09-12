//响应HTTP请求的抽象类
#pragma once
#include <string>
#include <sys/types.h>

class http_head
{
private:
    int status;
    long long length;
    std::string type;

public:
    http_head(int status, long long length, std::string type);
    ssize_t send(int fd);
};