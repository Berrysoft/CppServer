//把HTTP请求的URL分成三部分
#pragma once
#include <string>
#include <string_view>

struct http_url
{
    std::string module;
    std::string command;
    std::string args;
};

http_url get_url_from_string(const std::string& url);
