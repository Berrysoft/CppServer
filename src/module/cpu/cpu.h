//输出CPU、时间与进程信息。
#pragma once
#include "../response.h"

class cpu_response : public response
{
public:
    cpu_response(const http_request& request) : response(request) {}
    ~cpu_response() {}
    ssize_t send(int fd);
};
