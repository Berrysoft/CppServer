#include "http_request.h"
#include <sstream>
#include <sys/socket.h>

using namespace std;

http_request http_request::parse(int fd)
{
    http_request result;
    char buffer[4096];
    ssize_t len = recv(fd, buffer, sizeof(buffer), 0);
    istringstream iss(buffer);
    iss >> result.m_method >> result.m_url >> result.m_version;
    while (len >= (long)sizeof(buffer))
    {
        len = recv(fd, buffer, sizeof(buffer), 0);
    }
    return result;
}
