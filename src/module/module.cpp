#include "module.h"
#include <dlfcn.h>

using namespace std;

typedef void* (*get_handle)(void*);

module::module() : handle(nullptr)
{
}

module::~module()
{
    close();
}

bool module::open(string name)
{
    handle = dlopen(name.c_str(), RTLD_LAZY);
    return handle;
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

void module::close()
{
    if (handle)
    {
        dlclose(handle);
        handle = nullptr;
    }
}
