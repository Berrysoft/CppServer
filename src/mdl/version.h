#pragma once
#include "response.h"

class version_response : public response
{
public:
    virtual ~version_response() {}
    virtual ssize_t send(int fd);
};
