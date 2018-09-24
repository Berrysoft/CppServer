#include <http/http_url.h>
#include <regex>

using namespace std;

const regex module_regex("/([^/]*)/(\\S*)");
const regex args_regex("(\\S*)\\?(\\S*)");

http_url get_url_from_string(std::string url)
{
    http_url result;
    auto r1 = *(sregex_iterator(url.begin(), url.end(), module_regex));
    if (r1.size() > 2)
    {
        result.module = r1[1];
        string suf = r1[2];
        auto r2 = *(sregex_iterator(suf.begin(), suf.end(), args_regex));
        if (r2.size() < 3)
        {
            result.command = suf;
        }
        else
        {
            result.command = r2[1];
            result.args = r2[2];
        }
    }
    return result;
}
