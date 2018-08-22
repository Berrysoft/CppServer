//读取并输出文件内容，或者动态生成主页。
#pragma once
#include <string>
#include "../response.h"

class file_response : public response
{
private:
    std::string filename;
public:
    virtual ~file_response(){}
    file_response(std::string filename);

    virtual ssize_t send(int fd);
};
