//一个简易Markdown引擎.
#pragma once
#include <module/response.h>
#include <string>

class markdown_response : public response
{
private:
    std::string filename;
    bool file_exists;

public:
    markdown_response(const http_request& request, std::string filename);
    ~markdown_response() override {}

    ssize_t send(int fd) override;
};
