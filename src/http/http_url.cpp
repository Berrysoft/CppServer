#include <http/http_url.h>
#include <regex>

using namespace std;

const regex module_regex("/([^/\\f\\n\\r\\t\\v]*)/(\\S*)");
const regex args_regex("(\\S*)\\?(\\S*)");

http_url get_url_from_string(std::string url)
{
    http_url result;
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
    return result;
}
