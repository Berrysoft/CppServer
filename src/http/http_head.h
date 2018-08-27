//响应HEAD请求的类。
#pragma once
#include "http_response.h"
#include <cstdio>

class http_head : public http_response
{
public:
    virtual ssize_t send(int fd);
};
