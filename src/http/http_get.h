//响应GET请求的类。
#pragma once
#include "http_request.h"
#include "http_response.h"
#include <cstdio>
#include <map>
#include <string>

class http_get : public http_response
{
private:
    std::string url;
    std::string version;
    const std::map<std::string, std::string>& modules;

public:
    http_get(const http_request& request, const std::map<std::string, std::string>& modules);
    virtual ssize_t send(int fd);
};
