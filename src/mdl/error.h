#pragma once
#include "response.h"

class error_response : public response
{
public:
    virtual ~error_response() {}
    virtual bool supports(const char *version) { return true; }

    virtual int length();
    virtual int status() { return 400; }
    virtual ssize_t send(int fd);
};
