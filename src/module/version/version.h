//输出系统版本和内存信息。
#pragma once
#include "../response.h"

class version_response : public response
{
public:
    version_response(const http_request& request) : response(request) {}
    virtual ~version_response() {}
    virtual ssize_t send(int fd);
};
