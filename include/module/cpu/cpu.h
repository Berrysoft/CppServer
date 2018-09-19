//输出CPU、时间与进程信息。
#pragma once
#include <module/response.h>

class cpu_response : public response
{
public:
    cpu_response(const http_request& request) : response(request) {}
    ~cpu_response() override {}
    ssize_t send(int fd) override;
};
