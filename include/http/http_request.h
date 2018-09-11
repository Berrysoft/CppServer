//解析HTTP请求
#pragma once
#include <string>
#include <optional>

class http_request
{
private:
    std::string m_method;
    std::string m_url;
    double m_version;

    std::string m_content;

public:
    const std::string& method() const { return m_method; }
    const std::string& url() const { return m_url; }
    const double& version() const { return m_version; }

    const std::string& content() const { return m_content; }

    static std::optional<http_request> parse(int fd);
};
