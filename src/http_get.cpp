#include "http_get.h"
#include <cstring>
#include <sstream>
#include <sys/socket.h>
#include <unistd.h>
#include "module.h"
#include "mdl/response.h"
#include "mdl/html_writer.h"

using namespace std;

http_get::http_get(string request, map<string, string> &modules) : modules(modules)
{
    istringstream iss(request);
    iss >> url >> version;
    if (url.length() > 0)
    {
        if (url[0] == '/')
        {
            url = url.substr(1);
        }
        if (url.length() > 0)
        {
            if (url.back() == '/')
            {
                url = url.substr(0, url.length() - 1);
            }
        }
    }
}

unique_ptr<response> get_command_response(string command, string response_command, map<string, string> &modules, module &m)
{
    map<string, string>::iterator it;
    it = modules.find(command);
    if (it != modules.end())
    {
        m.open(it->second);
        return m.get_response(response_command);
    }
    return nullptr;
}

unique_ptr<response> deal_commands(string command, map<string, string> &modules, module &m)
{
    if (command.length() == 0)
        command = "file/";
    string temp;
    int index = command.find_first_of('/');
    if (index > 0)
    {
        temp = command.substr(index + 1);
        command = command.substr(0, index);
    }
    else if (index == 0)
    {
        temp = command;
        command = string();
    }
    unique_ptr<response> result = get_command_response(command, temp, modules, m);
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
        unique_ptr<response> res = nullptr;
        res = deal_commands(url, modules, m);
        if (res && (!res->supports(version)))
        {
            res = deal_commands("error", modules, m);
        }

        const char head[] = "HTTP/1.1 %d %s\r\nServer: Berrysoft.Linux.Cpp.Server\r\nContent-Type: text/html;charset=UTF-8\r\nConnection: keep-alive\r\n%s\r\n\r\n";
        char realhead[256];
        char length[64];
        int res_status;
        if (res)
        {
            int res_length = res->length();
            if (res_length < 0)
            {
                strcpy(length, "Transfer-Encoding: chunked");
            }
            else
            {
                sprintf(length, "Content-Length: %d", res_length);
            }
            res_status = res->status();
        }
        else
        {
            length[0] = '\0';
            res_status = 400;
        }
        switch (res_status)
        {
        case 200:
            sprintf(realhead, head, 200, "OK", length);
            break;
        case 400:
            sprintf(realhead, head, 400, "Bad Request", length);
            break;
        case 404:
            sprintf(realhead, head, 404, "Not Found", length);
            break;
        }
        IF_NEGATIVE_EXIT(::send(fd, realhead, strlen(realhead), 0));
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
    RETURN_RESULT;
}
