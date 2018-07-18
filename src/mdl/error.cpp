#include "error.h"
#include <sys/socket.h>

const char error_html[] = "<!DOCTYPE html><html><head><meta http-equiv=\"Content-type\" content=\"text/html;charset=UTF-8\"><title>大作业-错误</title></head><body><h1>出错啦！</h1><p>我们找不到请求的文件或者请求无效，请返回到上一页。</p></body></html>";

int error_response::length()
{
    return sizeof(error_html) - 1;
}

ssize_t error_response::send(int fd)
{
    return ::send(fd, error_html, length(), 0);
}

void* get_instance_response(const char* command)
{
    return new error_response();
}
