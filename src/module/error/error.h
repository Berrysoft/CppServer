#pragma once
#include "../response.h"

class error_response : public response
{
public:
    error_response(const http_request& request) : response(request) {}
    ~error_response() {}
    long long length();
    int status() { return 400; }
    ssize_t send(int fd);
};
