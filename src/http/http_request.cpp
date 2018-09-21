#include <http/http_request.h>
#include <sf/format.hpp>
#include <sstream>
#include <sys/socket.h>

using namespace std;
using namespace sf;

optional<http_request> http_request::parse(int fd)
{
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
    optional<http_request> result = make_optional<http_request>();
    scan(ss, "{0}{1}HTTP/{2}", result->m_method, result->m_url, result->m_version);
    string line;
    getline(ss, line);
    do
    {
        getline(ss, line);
        if (line.back() == '\r')
        {
            line.pop_back();
        }
    } while (!line.empty());
    ostringstream oss;
    while (!ss.eof())
    {
        ss.read(buffer, sizeof(buffer));
        oss.write(buffer, ss.gcount());
    }
    result->m_content = oss.str();
    return result;
}
