#include <http/http_url.h>

using namespace std;

http_url get_url_from_string(std::string url)
{
    http_url result;
    if (!url.empty())
    {
        url.erase(url.begin());
        size_t index = url.find_first_of('/');
        if (index != string::npos)
        {
            if (index + 1 >= url.length())
            {
                url.pop_back();
                result.module = url;
            }
            else
            {
                result.module = url.substr(0, index);
                url = url.substr(index + 1);
                index = url.find_first_of('?');
                if (index != string::npos)
                {
                    if (index + 1 >= url.length())
                    {
                        url.pop_back();
                        result.command = url;
                    }
                    else
                    {
                        result.command = url.substr(0, index);
                        result.args = url.substr(index + 1);
                    }
                }
                else
                {
                    result.command = url;
                }
            }
        }
        else
        {
            result.module = url;
        }
    }
    return result;
}
