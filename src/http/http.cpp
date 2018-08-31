#include "http.h"
#include "../html/html_writer.h"
#include "../http/http_url.h"
#include "../module/module.h"
#include "../module/read_modules.h"
#include "../module/response.h"
#include "http_head.h"
#include <sstream>
#include <sys/socket.h>

using namespace std;

http::http()
{
    refresh_modules();
}

void http::refresh_modules()
{
    vector<string> lines = read_modules_file();
    modules.clear();
    for (string& line : lines)
    {
        istringstream iss(line);
        string key, module_name;
        iss >> key >> module_name;
        modules.emplace(key, module_name);
    }
}

ssize_t http::send(int fd, const http_request& request)
{
    INIT_RESULT_AND_TEMP;
    string mod = get_url_from_string(request.url()).module;
    if (mod.empty())
    {
        mod = "file";
    }
    module m;
    {
        unique_ptr<response> res;
        auto it = modules.find(mod);
        if (it != modules.end() && m.open(it->second))
        {
            res = m.get_response(request);
        }
        int res_status = 400;
        string content_type;
        long long res_length = 0;
        if (res)
        {
            res_status = res->status();
            res_length = res->length();
            content_type = res->type();
        }
        http_head head(res_status, res_length, content_type);
        IF_NEGATIVE_EXIT(head.send(fd));
        if (res && request.method() != "HEAD")
        {
            IF_NEGATIVE_EXIT(res->send(fd));
        }
    }
    RETURN_RESULT;
}
