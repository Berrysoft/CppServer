#pragma once
#include "response.h"

class cpu_response : public response
{
public:
    virtual ~cpu_response() {}
    virtual ssize_t send(int fd);
};
