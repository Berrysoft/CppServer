#include "module.h"
#include <cstring>
#include <dlfcn.h>

using namespace std;

typedef void *(*get_handle)(const char *);

module::module(const char *name)
{
    strcpy(filename, name);
}

bool module::open()
{
    handle = dlopen(filename, RTLD_LAZY);
    if (!handle)
    {
        return false;
    }
    dlerror();
    return true;
}

response *module::get_response(const char* command)
{
    get_handle f = (get_handle)dlsym(handle, "get_instance_response");
    if (dlerror() != nullptr)
        return nullptr;
    response *result = (response*)f(command);
    return result;
}

void module::close()
{
    dlclose(handle);
    dlerror();
}
