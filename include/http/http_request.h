//解析HTTP请求
#pragma once
#include <optional>
#include <string>
#include <http/http_url.h>

class http_request
{
private:
    std::string m_method;
    std::string m_url;
    http_url m_split_url;
    double m_version;

    std::string m_content;

public:
    const std::string& method() const { return m_method; }
    const std::string& url() const { return m_url; }
    const http_url& split_url() const { return m_split_url; }
    const double& version() const { return m_version; }

    const std::string& content() const { return m_content; }

    static std::optional<http_request> parse(int fd);
};
