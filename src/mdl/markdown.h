//一个简易Markdown引擎，为../README.md文件生成HTML。
//注意为了方便，这里用的是绝对路径。
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
