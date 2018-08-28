#include "http_request.h"
#include <sstream>
#include <sys/socket.h>

using namespace std;

optional<http_request> http_request::parse(int fd)
{
    http_request result;
    stringstream ss;
    char buffer[4096];
    ssize_t len;
    do
    {
        len = recv(fd, buffer, sizeof(buffer), 0);
        if (len < 0)
            return nullopt;
        ss.write(buffer, len);
    } while (len >= (long)sizeof(buffer));
    ss >> result.m_method >> result.m_url >> result.m_version;
    string line;
    do
    {
        getline(ss, line);
    } while (!line.empty());
    ostringstream oss;
    while (!ss.eof())
    {
        ss.read(buffer, sizeof(buffer));
        oss.write(buffer, ss.gcount());
    }
    result.m_content = oss.str();
    return result;
}
