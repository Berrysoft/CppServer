#include "http_get.h"
#include <cstring>
#include <sstream>
#include <sys/socket.h>
#include <unistd.h>
#include "module.h"
#include "mdl/response.h"

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

string status_des(int status)
{
    switch(status)
    {
    case 200:
        return "OK";
    case 400:
        return "Bad Request";
    case 404:
        return "Not Found";
    default:
        return string();
    }
}

ssize_t http_get::send(int fd)
{
    module m;
    ssize_t result = 0, t;
    {
        unique_ptr<response> res = deal_commands(url, modules, m);
        if (res && (!res->supports(version)))
        {
            res = deal_commands("error", modules, m);
        }
        int res_status;
        if (res)
        {
            res_status = res->status();
        }
        else
        {
            res_status = 400;
        }
        ostringstream head;
        head << "HTTP/1.1 " << res_status << ' ' << status_des(res_status) << "\r\nServer: Berrysoft.Linux.Cpp.Server\r\nContent-Type: text/html;charset=UTF-8\r\nConnection: keep-alive\r\n";
        if(res)
        {
            int res_length = res->length();
            if (res_length < 0)
            {
                head << "Transfer-Encoding: chunked\r\n\r\n";
            }
            else
            {
                head << "Content-Length: " << res_length << "\r\n\r\n";
            }
        }
        string realhead = head.str();
        t = ::send(fd, realhead.c_str(), realhead.length(), 0);
        if (t < 0)
        {
            result = -1;
        }
        else
        {
            result += t;
        }
        if (res)
        {
            t = res->send(fd);
            if (t < 0)
            {
                result = -1;
            }
            else
            {
                result += t;
            }
        }
    }
    return result;
}