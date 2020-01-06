//响应HTTP请求的抽象类
#pragma once
#include <string>
#include <string_view>
#include <sys/types.h>

class http_head
{
private:
    std::int32_t status;
    std::int64_t length;
    std::string type;

public:
    http_head(std::int32_t status, std::int64_t length, std::string_view type);
    ssize_t send(int fd);
};
