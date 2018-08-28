#include "module.h"
#include <cstring>
#include <dlfcn.h>

using namespace std;

typedef void* (*get_handle)(void*);

module::module() : handle(nullptr)
{
}

module::~module()
{
    if (handle)
    {
        dlclose(handle);
        handle = nullptr;
    }
}

void module::open(string name)
{
    handle = dlopen(name.c_str(), RTLD_LAZY);
}

unique_ptr<response> module::get_response(const http_request& request)
{
    if (handle)
    {
        get_handle f = (get_handle)dlsym(handle, "get_instance_response");
        return unique_ptr<response>((response*)f((void*)&request));
    }
    return nullptr;
}
