//解析HTTP请求的类
#pragma once
#include "http_request.h"
#include "http_response.h"
#include <cstdio>
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
    std::unique_ptr<http_response> get_response(const http_request& request);
};
