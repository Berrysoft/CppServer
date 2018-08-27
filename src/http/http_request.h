//解析HTTP请求
#pragma once
#include <string>

class http_request
{
private:
    std::string m_method;
    std::string m_url;
    std::string m_version;

public:
    const std::string& method() const { return m_method; }
    const std::string& url() const { return m_url; }
    const std::string& version() const { return m_version; }

    static http_request parse(int fd);
};
