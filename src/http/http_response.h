//响应HTTP请求的抽象类
#pragma once
#include <cstdio>
#include <string>

class http_response
{
public:
    virtual ssize_t send(int fd) = 0;
};

class http_response_head
{
private:
    int status;
    long long length;
    std::string type;

public:
    http_response_head(int status, long long length, std::string type);
    ssize_t send(int fd);
};
