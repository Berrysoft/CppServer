#include <dlfcn.h>
#include <module/module.h>
#include <module/response.h>

using namespace std;

module::module() : handle(nullptr)
{
}

module::~module()
{
    destory();
    close();
}

bool module::open(string_view name)
{
    handle = dlopen(name.data(), RTLD_LAZY);
    return handle;
}

template <typename T, typename... Args>
T invoke_module(void* handle, const char* name, T&& def, Args... args)
{
    using F = T (*)(Args...);
    if (handle)
    {
        F f = (F)dlsym(handle, name);
        if (f)
        {
            return f(args...);
        }
    }
    return forward<T>(def);
}

bool module::init(const http_request& request)
{
    init_response_arg arg{ request.method().c_str(), request.split_url().module.c_str(), request.split_url().command.c_str(), request.split_url().args.c_str(), request.content().c_str(), request.version() };
    return invoke_module<int32_t>(handle, "res_init", 0, &arg) == 0;
}

int32_t module::status()
{
    return invoke_module<int32_t>(handle, "res_status", 200);
}

int64_t module::length()
{
    return invoke_module<int64_t>(handle, "res_length", -1);
}

const char* module::type()
{
    return invoke_module<const char*>(handle, "res_type", "text/html");
}

ssize_t module::send(int fd)
{
    return invoke_module<ssize_t>(handle, "res_send", -1, fd);
}

bool module::destory()
{
    return invoke_module<int32_t>(handle, "res_destory", 0) == 0;
}

void module::close()
{
    if (handle)
    {
        dlclose(handle);
        handle = nullptr;
    }
}
