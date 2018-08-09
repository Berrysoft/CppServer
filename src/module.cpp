#include "module.h"
#include <cstring>
#include <dlfcn.h>

using namespace std;

typedef void *(*get_handle)(const char *);

module::module(string name) : filename(move(name))
{
}

bool module::open()
{
    handle = dlopen(filename.c_str(), RTLD_LAZY);
    if (!handle)
    {
        return false;
    }
    dlerror();
    return true;
}

unique_ptr<response> module::get_response(string command)
{
    get_handle f = (get_handle)dlsym(handle, "get_instance_response");
    if (dlerror() != nullptr)
        return nullptr;
    return unique_ptr<response>((response *)f(command.c_str()));
}

void module::close()
{
    if (handle)
    {
        dlclose(handle);
        dlerror();
        handle = nullptr;
    }
}
