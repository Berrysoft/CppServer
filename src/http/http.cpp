#include "http.h"
#include "../html/html_writer.h"
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

string get_command(const string& full_url)
{
    string url = full_url;
    if (url.length() > 0)
    {
        if (url[0] == '/')
        {
            url.erase(url.begin());
        }
    }
    if (url.length() > 0)
    {
        if (url.back() == '/')
        {
            url.erase(--url.end());
        }
    }
    else
    {
        url = "file";
    }
    return url;
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
    unique_ptr<response> result;
    size_t index = command.find_first_of('/');
    string temp;
    if (index != string::npos)
    {
        temp = command.substr(index + 1);
        command = command.substr(0, index);
    }
    if (command.length() > 0)
    {
        result = get_command_response(command, request, modules, m);
    }
    if (!result)
    {
        result = get_command_response("error", request, modules, m);
    }
    return result;
}

ssize_t http::send(int fd, const http_request& request)
{
    string url = get_command(request.url());
    module m;
    INIT_RESULT_AND_TEMP;
    {
        unique_ptr<response> res = deal_commands(url, request, modules, m);
        if (res && (!res->supports()))
        {
            res = deal_commands("error", request, modules, m);
        }
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
