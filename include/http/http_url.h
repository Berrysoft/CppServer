//把HTTP请求的URL分成三部分
#pragma once
#include <string>

struct http_url
{
    std::string module;
    std::string command;
    std::string args;
};

http_url get_url_from_string(std::string url);
