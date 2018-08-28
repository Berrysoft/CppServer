#pragma once
#include "../response.h"

class error_response : public response
{
public:
    error_response(const http_request& request) : response(request) {}
    virtual ~error_response() {}
    virtual bool supports(std::string) { return true; }

    virtual long long length();
    virtual int status() { return 404; }
    virtual ssize_t send(int fd);
};
