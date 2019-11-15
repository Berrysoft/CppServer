#include <http/http_request.h>
#include <regex>
#include <sf/format.hpp>
#include <sstream>
#include <sys/socket.h>

using namespace std;
using namespace sf;

const regex module_regex("/([^/\\f\\n\\r\\t\\v]*)/(\\S*)");
const regex args_regex("(\\S*)\\?(\\S*)");

void get_url_from_string(const string& url, http_request& result)
{
    smatch r1;
    if (regex_match(url, r1, module_regex))
    {
        result.module = r1[1].str();
        string suf = r1[2].str();
        smatch r2;
        if (regex_match(suf, r2, args_regex))
        {
            result.command = r2[1].str();
            result.args = r2[2].str();
        }
        else
        {
            result.command = suf;
        }
    }
}

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
    scan(ss, "{0}{1}HTTP/{2}", result->method, url, result->version);
    get_url_from_string(url, *result);
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
        result->content = ss.str().substr(index);
    }
    return result;
}
