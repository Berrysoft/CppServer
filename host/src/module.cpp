#include <filesystem>
#include <module/module.h>
#include <module/response.h>

using namespace std;
using namespace std::filesystem;

static const string module_prefix{ "lib" };
static const path module_extension{ ".so" };

static path get_full_path(path name)
{
    if (name != name.filename()) return name;
    if (!string_view{ name.c_str() }.starts_with(module_prefix))
    {
        name = module_prefix + name.string();
    }
    name.replace_extension(module_extension);
    return name;
}

module::module(string_view name, const http_request& request) : handle(nullptr)
{
    handle.reset(dlopen(get_full_path(name).c_str(), RTLD_LAZY));
    if (!init(request))
        handle = nullptr;
}

template <typename T>
struct module_helper
{
    template <typename... Args>
    static T invoke(const module_ptr& handle, const char* name, T&& def, Args... args)
    {
        using F = T (*)(Args...);
        if (handle)
        {
            F f = (F)dlsym(handle.get(), name);
            if (f)
            {
                return f(args...);
            }
        }
        return forward<T>(def);
    }
};

template <>
struct module_helper<void>
{
    template <typename... Args>
    static void invoke(const module_ptr& handle, const char* name, Args... args)
    {
        using F = void (*)(Args...);
        if (handle)
        {
            F f = (F)dlsym(handle.get(), name);
            if (f)
            {
                f(args...);
            }
        }
    }
};

bool module::init(const http_request& request)
{
    init_response_arg arg{ request.method.c_str(), request.module.c_str(), request.command.c_str(), request.args.c_str(), request.content.c_str(), request.version };
    return module_helper<int32_t>::invoke(handle, "res_init", -1, &arg) == 0;
}

int32_t module::status() { return module_helper<int32_t>::invoke(handle, "res_status", 200); }

int64_t module::length() { return module_helper<int64_t>::invoke(handle, "res_length", -1); }

string module::type() { return module_helper<const char*>::invoke(handle, "res_type", "text/html"); }

ssize_t module::send(int fd) { return module_helper<ssize_t>::invoke(handle, "res_send", -1, fd); }

void module::destory() { module_helper<void>::invoke(handle, "res_destory"); }
