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
    string url;
    scan(ss, "{0}{1}HTTP/{2}", result->m_method, url, result->m_version);
    result->m_split_url = get_url_from_string(url);
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
    if (!ss.eof())
    {
        auto index = ss.tellg();
        result->m_content = ss.str().substr(index);
    }
    return result;
}
