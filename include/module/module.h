//模块类，用于动态加载模块。
#pragma once
#include <functional>
#include <http/http_request.h>
#include <memory>
#include <string>
#include <string_view>

class module
{
private:
    void* handle;

public:
    module() : handle(nullptr) {}
    ~module() { destory(); }

    bool open(std::string_view name);
    bool init(const http_request& request);
    std::int32_t status();
    std::int64_t length();
    const char* type();
    ssize_t send(int fd);
    bool destory();
    void close();
};

struct module_guard
{
private:
    module& m;
    bool m_inited;

public:
    module_guard(module& m, const http_request& req) : m(m) { m_inited = m.init(req); }
    ~module_guard() { m.destory(); }

    constexpr bool inited() const noexcept { return m_inited; }
};
