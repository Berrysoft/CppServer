//响应HEAD请求的类。
#pragma once
#include <cstdio>
#include "http_response.h"

class http_head : public http_response
{
public:
    virtual ssize_t send(int fd);
};
