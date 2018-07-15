#include "html_writer.h"
#include <cstdio>
#include <cstring>
#include <sstream>
#include <sys/socket.h>

using namespace std;

ssize_t html_writer::write_head(string title)
{
    ssize_t result = 0;
    ssize_t t;
    const char head_start[] = "<!DOCTYPE html>\n<html>\n<head>\n<meta http-equiv=\"Content-type\" content=\"text/html;charset=UTF-8\">\n<title>%s</title>\n<link rel=\"stylesheet\" href=\"https://cdn.jsdelivr.net/npm/katex@0.10.0-alpha/dist/katex.min.css\" integrity=\"sha384-BTL0nVi8DnMrNdMQZG1Ww6yasK9ZGnUxL1ZWukXQ7fygA1py52yPp9W4wrR00VML\" crossorigin=\"anonymous\">\n<style>";
    const char head_end[] = "\n</style>\n</head>\n";
    char buffer[4096];
    memset(buffer, 0, sizeof(buffer));
    int len = sprintf(buffer, head_start, title.c_str());
    t = send(fd, buffer, len, 0);
    if (t < 0)
        return t;
    result += t;
    FILE *fcss = fopen("style.css", "r");
    if (fcss)
    {
        while (len = fread(buffer, sizeof(char), sizeof(buffer), fcss))
        {
            t = send(fd, buffer, len, 0);
            if (t < 0)
                return t;
            result += t;
        }
        fclose(fcss);
    }
    t = send(fd, head_end, sizeof(head_end) - 1, 0);
    if (t < 0)
        return t;
    result += t;
    return result;
}

ssize_t html_writer::write_spe(string spe, string text)
{
    ostringstream oss;
    oss << "<" << spe << ">" << text << "</" << spe << ">";
    string s = oss.str();
    return send(fd, s.c_str(), s.length(), 0);
}

ssize_t html_writer::write_h1(string title)
{
    return write_spe("h1", title);
}

ssize_t html_writer::write_p(string text)
{
    return write_spe("p", text);
}

ssize_t html_writer::write_h2(string title)
{
    return write_spe("h2", title);
}

ssize_t html_writer::write_h3(string title)
{
    return write_spe("h3", title);
}

ssize_t html_writer::write_ul(vector<string> texts)
{
    ssize_t result = 0;
    ssize_t t;
    t = send(fd, "<ul>", 4, 0);
    if (t < 0)
        return t;
    result += t;
    for (string &text : texts)
    {
        t = send(fd, "<li>", 4, 0);
        if (t < 0)
            return t;
        result += t;
        t = send(fd, text.c_str(), text.length(), 0);
        if (t < 0)
            return t;
        result += t;
        t = send(fd, "</li>", 5, 0);
        if (t < 0)
            return t;
        result += t;
    }
    t = send(fd, "</ul>", 5, 0);
    if (t < 0)
        return t;
    result += t;
    return result;
}

ssize_t html_writer::write_table_start(vector<string> texts)
{
    ssize_t result = 0;
    ssize_t t;
    const char table_start[] = "<table><thead><tr>";
    const char thead_end[] = "</tr></thead><tbody>";
    t = send(fd, table_start, sizeof(table_start) - 1, 0);
    if (t < 0)
        return t;
    result += t;
    for (string &text : texts)
    {
        t = send(fd, "<th>", 4, 0);
        if (t < 0)
            return t;
        result += t;
        t = send(fd, text.c_str(), text.length(), 0);
        if (t < 0)
            return t;
        result += t;
        t = send(fd, "</th>", 5, 0);
        if (t < 0)
            return t;
        result += t;
    }
    t = send(fd, thead_end, sizeof(thead_end) - 1, 0);
    if (t < 0)
        return t;
    result += t;
    return result;
}

ssize_t html_writer::write_tr(vector<string> texts)
{
    ssize_t result = 0;
    ssize_t t;
    t = send(fd, "<tr>", 4, 0);
    if (t < 0)
        return t;
    result += t;
    for (string &text : texts)
    {
        t = send(fd, "<td>", 4, 0);
        if (t < 0)
            return t;
        result += t;
        t = send(fd, text.c_str(), text.length(), 0);
        if (t < 0)
            return t;
        result += t;
        t = send(fd, "</td>", 5, 0);
        if (t < 0)
            return t;
        result += t;
    }
    t = send(fd, "</tr>", 5, 0);
    if (t < 0)
        return t;
    result += t;
    return result;
}

ssize_t html_writer::write_table_end()
{
    const char table_end[] = "</tbody></table>";
    return send(fd, table_end, sizeof(table_end) - 1, 0);
}

ssize_t html_writer::write_pre_code_start()
{
    const char code_start[] = "<pre><code>";
    return send(fd, code_start, sizeof(code_start) - 1, 0);
}

ssize_t html_writer::write_pre_code_end()
{
    const char code_end[] = "</code></pre>";
    return send(fd, code_end, sizeof(code_end) - 1, 0);
}

ssize_t html_writer::write_code_start()
{
    const char code_start[] = "<code>";
    return send(fd, code_start, sizeof(code_start) - 1, 0);
}

ssize_t html_writer::write_code_end()
{
    const char code_end[] = "</code>";
    return send(fd, code_end, sizeof(code_end) - 1, 0);
}

ssize_t html_writer::write_xmp_start()
{
    const char xmp_start[] = "<xmp>";
    return send(fd, xmp_start, sizeof(xmp_start) - 1, 0);
}

ssize_t html_writer::write_xmp_end()
{
    const char xmp_end[] = "</xmp>";
    return send(fd, xmp_end, sizeof(xmp_end) - 1, 0);
}

ssize_t html_writer::write_end()
{
    const char html_end[] = "</html>";
    return send(fd, html_end, sizeof(html_end) - 1, 0);
}
