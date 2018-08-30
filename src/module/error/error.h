//生成错误页面
//支持HTTP/1.0
#pragma once
#include "../response.h"

class error_response : public response
{
public:
    error_response(const http_request& request) : response(request) {}
    ~error_response() {}
    long long length();
    int status() { return 404; }
    ssize_t send(int fd);
};
