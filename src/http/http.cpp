#include "http.h"
#include "../html/html_writer.h"
#include "../http/http_url.h"
#include "../module/module.h"
#include "../module/read_modules.h"
#include "../module/response.h"
#include "http_head.h"
#include <sstream>

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

unique_ptr<response> get_command_response(string command, const http_request& request, const map<string, string>& modules, module& m)
{
    map<string, string>::const_iterator it;
    it = modules.find(command);
    if (it != modules.end())
    {
        m.open(it->second);
        return m.get_response(request);
    }
    return nullptr;
}

unique_ptr<response> deal_commands(string command, const http_request& request, const map<string, string>& modules, module& m)
{
    if (command.empty())
    {
        command = "file";
    }
    unique_ptr<response> result = get_command_response(command, request, modules, m);
    if (!result)
    {
        result = get_command_response("error", request, modules, m);
    }
    return result;
}

ssize_t http::send(int fd, const http_request& request)
{
    string mod = get_url_from_string(request.url()).module;
    module m;
    INIT_RESULT_AND_TEMP;
    {
        unique_ptr<response> res = deal_commands(mod, request, modules, m);
        int res_status;
        string content_type;
        long long res_length;
        if (res)
        {
            res_status = res->status();
            res_length = res->length();
            content_type = res->type();
        }
        else
        {
            res_status = 400;
        }
        http_head head(res_status, res_length, content_type);
        IF_NEGATIVE_EXIT(head.send(fd));
        if (res)
        {
            IF_NEGATIVE_EXIT(res->send(fd));
        }
    }
    RETURN_RESULT;
}
