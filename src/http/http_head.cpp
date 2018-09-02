#include "http_head.h"
#include <fmt/printf.h>
#include <sstream>
#include <sys/socket.h>

using namespace std;
using fmt::print;

http_head::http_head(int status, long long length, string type)
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

ssize_t http_head::send(int fd)
{
    ostringstream head;
    print(head, "HTTP/1.1 {0} {1}\r\nServer: Berrysoft.Linux.Cpp.Server\r\nContent-Charset=UTF-8\r\nConnection: keep-alive\r\n", status, status_des(status));
    if (!type.empty())
    {
        print(head, "Content-Type: {0}\r\n", type);
    }
    if (length < 0)
    {
        print(head, "Transfer-Encoding: chunked\r\n\r\n");
    }
    else
    {
        print(head, "Content-Length: {0}\r\n\r\n", length);
    }
    string realhead = head.str();
    return ::send(fd, realhead.c_str(), realhead.length(), 0);
}
