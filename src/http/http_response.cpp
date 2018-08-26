#include "http_response.h"
#include <sys/socket.h>
#include <sstream>

using namespace std;

http_response_head::http_response_head(int status, long long length, string type)
    : status(status), length(length), type(type)
{
}

string status_des(int status)
{
    switch (status)
    {
    case 200:
        return "OK";
    case 400:
        return "Bad Request";
    case 404:
        return "Not Found";
    default:
        return string();
    }
}

ssize_t http_response_head::send(int fd)
{
    ostringstream head;
    head << "HTTP/1.1 " << status << ' ' << status_des(status) << "\r\nServer: Berrysoft.Linux.Cpp.Server\r\nContent-Charset=UTF-8\r\nConnection: keep-alive\r\n";
    if (!type.empty())
    {
        head << "Content-Type: " << type << "\r\n";
    }
    if (length < 0)
    {
        head << "Transfer-Encoding: chunked\r\n\r\n";
    }
    else
    {
        head << "Content-Length: " << length << "\r\n\r\n";
    }
    string realhead = head.str();
    return ::send(fd, realhead.c_str(), realhead.length(), 0);
}
