#include "http.h"
#include <sstream>
#include "../module/read_modules.h"
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
        modules.emplace(key, module_name);
    }
}

unique_ptr<http_response> http::get_response(const http_request &request)
{
    const string &method = request.method();
    if (method == "GET")
    {
        return make_unique<http_get>(request, modules);
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
