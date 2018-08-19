//响应HTTP请求的抽象类
#pragma once
#include <cstdio>

class http_response
{
public:
    virtual ssize_t send(int fd) = 0;
};