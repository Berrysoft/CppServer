//解析HTTP请求
#pragma once
#include <optional>
#include <string>

struct http_request
{
    std::string method;
    std::string module;
    std::string command;
    std::string args;
    std::string content;
    double version;

    static std::optional<http_request> parse(int fd);
};
