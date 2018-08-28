//生成硬盘信息。
//由于开发环境WSL没有/proc/partitions文件，
//只能用statfs获取根目录信息（实际上是C盘的信息）。
#pragma once
#include "../response.h"

class disk_response : public response
{
public:
    disk_response(const http_request& request) : response(request) {}
    virtual ~disk_response() {}
    virtual ssize_t send(int fd);
};
