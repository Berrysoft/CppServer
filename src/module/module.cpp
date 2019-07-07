#include <dlfcn.h>
#include <module/module.h>

using namespace std;

using get_handle = void* (*)(void*);
using delete_handle = void (*)(void*);

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

module::response_ptr module::get_response(const http_request& request)
{
    if (handle)
    {
        get_handle f = (get_handle)dlsym(handle, "get_instance_response");
        delete_handle d = (delete_handle)dlsym(handle, "delete_instance_response");
        return module::response_ptr((response*)f((void*)&request), d);
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
