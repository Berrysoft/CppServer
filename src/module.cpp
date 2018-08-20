#include "module.h"
#include <cstring>
#include <dlfcn.h>

using namespace std;

typedef void *(*get_handle)(const char *);

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

unique_ptr<response> module::get_response(string command)
{
    if (handle)
    {
        get_handle f = (get_handle)dlsym(handle, "get_instance_response");
        char *e;
        if ((e = dlerror()) == nullptr)
            return unique_ptr<response>((response *)f(command.c_str()));
        else
            puts(e);
    }
    return nullptr;
}
