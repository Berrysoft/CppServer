//输出系统版本和内存信息。
#pragma once
#include <module/response.h>

class version_response : public response
{
public:
    version_response(const http_request& request) : response(request) {}
    ~version_response() override {}
    ssize_t send(int fd) override;
};
