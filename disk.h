#pragma once
#include "response.h"

class disk_response : public response
{
public:
    virtual ~disk_response() {}
    virtual ssize_t send(int fd);
};
