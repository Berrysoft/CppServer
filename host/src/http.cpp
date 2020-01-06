#include <html/html_writer.h>
#include <http/http.h>
#include <http/http_head.h>
#include <module/module.h>
#include <module/read_modules.h>
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
    string mod = request.module;
    if (mod.empty())
    {
        mod = "file";
    }
    module m;
    auto it = modules.find(mod);
    if (it != modules.end())
    {
        m = module(it->second, request);
    }
    int32_t res_status = 400;
    string content_type;
    int64_t res_length = 0;
    if (m)
    {
        res_status = m.status();
        res_length = m.length();
        content_type = m.type();
    }
    http_head head(res_status, res_length, content_type);
    IF_NEGATIVE_EXIT(head.send(fd));
    if (m && request.method != "HEAD")
    {
        IF_NEGATIVE_EXIT(m.send(fd));
    }
    RETURN_RESULT;
}
