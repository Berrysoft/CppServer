#include "html_content.h"
#include <cstring>
#include <sstream>
#include <sys/socket.h>
#include <unistd.h>
#include "mdl/response.h"

using namespace std;

html_content::html_content(const char *request)
{
    istringstream iss(request);
    iss >> method >> url;
    if (url.length() > 0)
    {
        if (url[0] == '/')
        {
            url = url.substr(1);
        }
        if (url.length() > 0)
        {
            if (url[url.length() - 1] == '/')
            {
                url = url.substr(0, url.length() - 1);
            }
        }
    }
}

response *get_command_response(string command, string response_command, map<string, module> &modules, module &m)
{
    map<string, module>::iterator it;
    it = modules.find(command);
    if (it != modules.end())
    {
        m = it->second;
        if (m.open())
        {
            response *res = m.get_response(response_command.c_str());
            if (res)
            {
                return res;
            }
            else
            {
                m.close();
                return nullptr;
            }
        }
        else
        {
            return nullptr;
        }
    }
    else
    {
        return nullptr;
    }
}

response *get_file_response(string command, map<string, module> &modules, int &resp, module &m)
{
    if (access(command.c_str(), 0) && command != "index.html")
    {
        resp = 404;
        command = "error.html";
    }
    else
    {
        resp = 200;
    }
    response *result = get_command_response("file", command, modules, m);
    if (result)
    {
        return result;
    }
    else
    {
        resp = 400;
        return nullptr;
    }
}

response *deal_commands(string command, map<string, module> &modules, int &resp, module &m)
{
    if (command.length() == 0)
        command = "index.html";
    if (command == "error")
    {
        return get_file_response("error.html", modules, resp, m);
    }
    else
    {
        response *result = get_command_response(command, string(), modules, m);
        if (result)
        {
            resp = 200;
            return result;
        }
        else
        {
            return get_file_response(command, modules, resp, m);
        }
    }
}

ssize_t html_content::send(int fd, map<string, module> &modules)
{
    int resp_value;
    response *res = nullptr;
    module m;
    if (method == "GET")
    {
        res = deal_commands(url, modules, resp_value, m);
    }
    else
    {
        res = deal_commands("error", modules, resp_value, m);
    }

    const char head[] = "HTTP/1.1 %d %s\r\nServer: Berrysoft.Linux.Cpp.Server\r\nContent-Type: text/html;charset=UTF-8\r\nTransfer-Encoding: chunked\r\n\r\n";
    char realhead[256];
    memset(realhead, 0, sizeof(realhead));
    switch (resp_value)
    {
    case 200:
        sprintf(realhead, head, 200, "OK");
        break;
    case 400:
        sprintf(realhead, head, 400, "Bad Request");
        break;
    case 404:
        sprintf(realhead, head, 404, "Not Found");
        break;
    }
    ssize_t result = ::send(fd, realhead, strlen(realhead), 0);
    if (res)
    {
        if (result < 0)
        {
            return result;
        }
        ssize_t t = res->send(fd);
        if (t < 0)
        {
            result = -1;
        }
        else
        {
            result += t;
            result += ::send(fd, "0\r\n\r\n", 5, 0);
        }
        delete res;
        m.close();
    }
    return result;
}
