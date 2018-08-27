//读取并输出文件内容，或者动态生成主页。
#pragma once
#include "../response.h"
#include <string>

class raw_response : public response
{
private:
    std::string filename;

public:
    virtual ~raw_response() {}
    raw_response(std::string filename);
    virtual std::string type();
    virtual ssize_t send(int fd);
};
