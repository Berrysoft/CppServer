#include "http.h"
#include <sstream>
#include "read_modules.h"
#include "http_get.h"
#include "http_head.h"

using namespace std;

http::http()
{
    refresh_modules();
}

void http::refresh_modules()
{
    vector<string> lines = read_modules_file();
    modules.clear();
    for (string &line : lines)
    {
        istringstream iss(line);
        string key, module_name;
        iss >> key >> module_name;
        modules.insert(map<string, string>::value_type(key, module_name));
    }
}

unique_ptr<http_response> http::get_response(const char *request)
{
    istringstream iss(request);
    string method, req;
    iss >> method;
    getline(iss, req);
    if (method == "GET")
    {
        return make_unique<http_get>(req, modules);
    }
    else if (method == "HEAD")
    {
        return make_unique<http_head>();
    }
    else
    {
        return nullptr;
    }
}
