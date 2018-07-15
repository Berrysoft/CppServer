#pragma once
#include <string>
#include "response.h"

class file_response : public response
{
private:
    std::string filename;
public:
    virtual ~file_response(){}
    file_response(const char *filename);

    virtual ssize_t send(int fd);
};
