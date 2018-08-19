//响应GET请求的类。
#pragma once
#include <cstdio>
#include <map>
#include <string>
#include "http_response.h"

class http_get : public http_response
{
private:
    std::string url;
    std::string version;
    std::map<std::string, std::string> &modules;
public:
    http_get(std::string request, std::map<std::string, std::string> &modules);
    virtual ssize_t send(int fd);
};
