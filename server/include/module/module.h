//模块类，用于动态加载模块。
#pragma once
#include <dlfcn.h>
#include <functional>
#include <http/http_request.h>
#include <memory>
#include <string>
#include <string_view>

struct module_ptr_deleter
{
    constexpr module_ptr_deleter() noexcept = default;
    void operator()(void* ptr) const
    {
        if (ptr) ::dlclose(ptr);
    }
};

using module_ptr = std::unique_ptr<void, module_ptr_deleter>;

class module
{
private:
    module_ptr handle;

    bool init(const http_request& request);
    void destory();

public:
    module() : handle(nullptr) {}
    module(std::string_view name, const http_request& request);
    module(module&& m) : handle(std::move(m.handle)) {}

    module& operator=(module&& m)
    {
        std::swap(handle, m.handle);
        return *this;
    }

    ~module() { destory(); }

    operator bool() const noexcept { return (bool)handle; }

    std::int32_t status();
    std::int64_t length();
    std::string type();
    ssize_t send(int fd);
};
