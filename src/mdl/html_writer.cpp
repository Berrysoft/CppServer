#include "html_writer.h"
#include <cstdio>
#include <cstring>
#include <sstream>
#include <sys/socket.h>

using namespace std;

ssize_t send_with_chunk(int fd, const void *buffer, size_t length, int flag)
{
    INIT_RESULT_AND_TEMP;
    char buf[9];
    int len = sprintf(buf, "%lX\r\n", length);
    IF_NEGATIVE_EXIT(send(fd, buf, len, flag));
    IF_NEGATIVE_EXIT(send(fd, buffer, length, flag));
    IF_NEGATIVE_EXIT(send(fd, "\r\n", 2, flag));
    RETURN_RESULT;
}

ssize_t html_writer::write_head(string title)
{
    INIT_RESULT_AND_TEMP;
    const char head_start[] = "<!DOCTYPE html>\n<html>\n<head>\n<meta http-equiv=\"Content-type\" content=\"text/html;charset=UTF-8\">\n<title>%s</title>\n<link rel=\"stylesheet\" href=\"https://cdn.jsdelivr.net/npm/katex@0.10.0-alpha/dist/katex.min.css\" integrity=\"sha384-BTL0nVi8DnMrNdMQZG1Ww6yasK9ZGnUxL1ZWukXQ7fygA1py52yPp9W4wrR00VML\" crossorigin=\"anonymous\">\n<style>\n";
    const char head_end[] = "\n</style>\n</head>\n";
    char buffer[4096];
    memset(buffer, 0, sizeof(buffer));
    int len = sprintf(buffer, head_start, title.c_str());
    IF_NEGATIVE_EXIT(send_with_chunk(fd, buffer, len, 0));
    FILE *fcss = fopen("style.css", "r");
    if (fcss)
    {
        while (len = fread(buffer, sizeof(char), sizeof(buffer), fcss))
        {
            IF_NEGATIVE_EXIT(send_with_chunk(fd, buffer, len, 0));
        }
        fclose(fcss);
    }
    IF_NEGATIVE_EXIT(send_with_chunk(fd, head_end, sizeof(head_end) - 1, 0));
    RETURN_RESULT;
}

ssize_t html_writer::write_spe(string spe, string text)
{
    ostringstream oss;
    oss << "<" << spe << ">" << text << "</" << spe << ">";
    string s = oss.str();
    return send_with_chunk(fd, s.c_str(), s.length(), 0);
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
    INIT_RESULT_AND_TEMP;
    IF_NEGATIVE_EXIT(send_with_chunk(fd, "<ul>", 4, 0));
    for (string &text : texts)
    {
        string te = "<li>" + text + "</li>";
        IF_NEGATIVE_EXIT(send_with_chunk(fd, te.c_str(), te.length(), 0));
    }
    IF_NEGATIVE_EXIT(send_with_chunk(fd, "</ul>", 5, 0));
    RETURN_RESULT;
}

ssize_t html_writer::write_table_start(vector<string> texts)
{
    INIT_RESULT_AND_TEMP;
    const char table_start[] = "<table><thead><tr>";
    const char thead_end[] = "</tr></thead><tbody>";
    IF_NEGATIVE_EXIT(send_with_chunk(fd, table_start, sizeof(table_start) - 1, 0));
    for (string &text : texts)
    {
        string te = "<th>" + text + "</th>";
        IF_NEGATIVE_EXIT(send_with_chunk(fd, te.c_str(), te.length(), 0));
    }
    IF_NEGATIVE_EXIT(send_with_chunk(fd, thead_end, sizeof(thead_end) - 1, 0));
    RETURN_RESULT;
}

ssize_t html_writer::write_tr(vector<string> texts)
{
    INIT_RESULT_AND_TEMP;
    IF_NEGATIVE_EXIT(send_with_chunk(fd, "<tr>", 4, 0));
    for (string &text : texts)
    {
        string te = "<td>" + text + "</td>";
        IF_NEGATIVE_EXIT(send_with_chunk(fd, te.c_str(), te.length(), 0));
    }
    IF_NEGATIVE_EXIT(send_with_chunk(fd, "</tr>", 5, 0));
    RETURN_RESULT;
}

ssize_t html_writer::write_table_end()
{
    const char table_end[] = "</tbody></table>";
    return send_with_chunk(fd, table_end, sizeof(table_end) - 1, 0);
}

ssize_t html_writer::write_pre_code_start()
{
    const char code_start[] = "<pre><code>";
    return send_with_chunk(fd, code_start, sizeof(code_start) - 1, 0);
}

ssize_t html_writer::write_pre_code_end()
{
    const char code_end[] = "</code></pre>";
    return send_with_chunk(fd, code_end, sizeof(code_end) - 1, 0);
}

ssize_t html_writer::write_code_start()
{
    const char code_start[] = "<code>";
    return send_with_chunk(fd, code_start, sizeof(code_start) - 1, 0);
}

ssize_t html_writer::write_code_end()
{
    const char code_end[] = "</code>";
    return send_with_chunk(fd, code_end, sizeof(code_end) - 1, 0);
}

ssize_t html_writer::write_xmp_start()
{
    const char xmp_start[] = "<xmp>";
    return send_with_chunk(fd, xmp_start, sizeof(xmp_start) - 1, 0);
}

ssize_t html_writer::write_xmp_end()
{
    const char xmp_end[] = "</xmp>";
    return send_with_chunk(fd, xmp_end, sizeof(xmp_end) - 1, 0);
}

ssize_t html_writer::write_end()
{
    const char html_end[] = "</html>";
    return send_with_chunk(fd, html_end, sizeof(html_end) - 1, 0);
}
