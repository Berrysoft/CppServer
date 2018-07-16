#pragma once
#include "response.h"
#include <string>

class markdown_response : public response
{
private:
    std::string filename;
public:
    virtual ~markdown_response() {}
    markdown_response(std::string filename);
    virtual ssize_t send(int fd);
};
