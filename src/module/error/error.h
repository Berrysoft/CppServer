#pragma once
#include "response.h"

class error_response : public response
{
public:
    virtual ~error_response() {}
    virtual bool supports(std::string) { return true; }

    virtual int length();
    virtual int status() { return 404; }
    virtual ssize_t send(int fd);
};