//解析HTTP请求
#pragma once
#include <http/http_url.h>
#include <optional>
#include <string>

class http_request
{
private:
    std::string m_method;
    http_url m_split_url;
    std::string m_content;
    double m_version;

public:
    constexpr const std::string& method() const noexcept { return m_method; }
    constexpr const http_url& split_url() const noexcept { return m_split_url; }
    constexpr const std::string& content() const noexcept { return m_content; }
    constexpr double version() const noexcept { return m_version; }

    static std::optional<http_request> parse(int fd);
};
