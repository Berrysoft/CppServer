#include "http_get.h"
#include <cstring>
#include <sstream>
#include <sys/socket.h>
#include <unistd.h>
#include "../module/module.h"
#include "../module/response.h"
#include "../html/html_writer.h"

using namespace std;

http_get::http_get(const http_request &request, const map<string, string> &modules) : modules(modules)
{
    url = request.url();
    version = request.version();
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
}

unique_ptr<response> get_command_response(string command, string response_command, const map<string, string> &modules, module &m)
{
    map<string, string>::const_iterator it;
    it = modules.find(command);
    if (it != modules.end())
    {
        m.open(it->second);
        return m.get_response(response_command);
    }
    return nullptr;
}

unique_ptr<response> deal_commands(string command, const map<string, string> &modules, module &m)
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
        result = get_command_response(command, temp, modules, m);
    }
    if (!result)
    {
        result = get_command_response("error", string(), modules, m);
    }
    return result;
}

ssize_t http_get::send(int fd)
{
    module m;
    INIT_RESULT_AND_TEMP;
    {
        unique_ptr<response> res = deal_commands(url, modules, m);
        if (res && (!res->supports(version)))
        {
            res = deal_commands("error", modules, m);
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
        http_response_head head(res_status, res_length, content_type);
        IF_NEGATIVE_EXIT(head.send(fd));
        if (res)
        {
            IF_NEGATIVE_EXIT(res->send(fd));
        }
    }
    RETURN_RESULT;
}
