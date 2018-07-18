//响应HTTP请求的类。
//类名有“html”是由于之前在这里直接回复HTML，后来分离了功能。
#pragma once
#include <cstdio>
#include <map>
#include <string>
#include "mdl/response.h"
#include "module.h"

class html_content
{
private:
    std::string method;
    std::string url;
    std::string version;
public:
    html_content(const char *request);

    ssize_t send(int fd, std::map<std::string, module> &modules);
};
