#pragma once
#include <cstdio>
#include <map>
#include <string>
#include "response.h"
#include "module.h"

class html_content
{
private:
    std::string method;
    std::string url;
public:
    html_content(const char *request);

    ssize_t send(int fd, std::map<std::string, module> &modules);
};
