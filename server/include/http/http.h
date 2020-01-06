//解析HTTP请求的类
#pragma once
#include <cstdio>
#include <http/http_head.h>
#include <http/http_request.h>
#include <map>
#include <memory>
#include <string>

class http
{
private:
    std::map<std::string, std::string> modules;

public:
    http();
    void refresh_modules();
    ssize_t send(int fd, const http_request& request);
};
