//读取并输出文件内容，或者动态生成主页。
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

    virtual bool supports(const char *version);
    virtual ssize_t send(int fd);
};
