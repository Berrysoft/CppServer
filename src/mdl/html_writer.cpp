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

ssize_t send_chunk_end(int fd, int flag)
{
    return send(fd, "0\r\n\r\n", 5, flag);
}

ssize_t html_writer::write_head(string title)
{
    INIT_RESULT_AND_TEMP;
    const char head_start[] = "<!DOCTYPE html><html><head><meta http-equiv=\"Content-type\" content=\"text/html;charset=UTF-8\"><title>";
    const char head_mid[] = "</title><link rel=\"stylesheet\" href=\"https://cdn.jsdelivr.net/npm/katex@0.10.0-alpha/dist/katex.min.css\" integrity=\"sha384-BTL0nVi8DnMrNdMQZG1Ww6yasK9ZGnUxL1ZWukXQ7fygA1py52yPp9W4wrR00VML\" crossorigin=\"anonymous\"><style>";
    const char head_end[] = "</style></head><body>";
    IF_NEGATIVE_EXIT(send_with_chunk(fd, head_start, sizeof(head_start) - 1, 0));
    IF_NEGATIVE_EXIT(send_with_chunk(fd, title.c_str(), title.length(), 0));
    IF_NEGATIVE_EXIT(send_with_chunk(fd, head_mid, sizeof(head_mid) - 1, 0));
    size_t len;
    char buffer[4096];
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
    oss << '<' << spe << '>' << text << "</" << spe << '>';
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

ssize_t html_writer::write_spe_start(string spe)
{
    string t = '<' + spe + '>';
    return send_with_chunk(fd, t.c_str(), t.length(), 0);
}

ssize_t html_writer::write_spe_end(string spe)
{
    string t = "</" + spe + '>';
    return send_with_chunk(fd, t.c_str(), t.length(), 0);
}

ssize_t html_writer::write_ul(vector<string> texts)
{
    INIT_RESULT_AND_TEMP;
    IF_NEGATIVE_EXIT(write_spe_start("ul"));
    for (string &text : texts)
    {
        IF_NEGATIVE_EXIT(write_spe("li", text));
    }
    IF_NEGATIVE_EXIT(write_spe_end("ul"));
    RETURN_RESULT;
}

ssize_t html_writer::write_p_start()
{
    return write_spe_start("p");
}

ssize_t html_writer::write_p_end()
{
    return write_spe_end("p");
}

ssize_t html_writer::write_table_start(vector<string> texts)
{
    INIT_RESULT_AND_TEMP;
    const char table_start[] = "<table><thead><tr>";
    const char thead_end[] = "</tr></thead><tbody>";
    IF_NEGATIVE_EXIT(send_with_chunk(fd, table_start, sizeof(table_start) - 1, 0));
    for (string &text : texts)
    {
        IF_NEGATIVE_EXIT(write_spe("th", text));
    }
    IF_NEGATIVE_EXIT(send_with_chunk(fd, thead_end, sizeof(thead_end) - 1, 0));
    RETURN_RESULT;
}

ssize_t html_writer::write_tr(vector<string> texts)
{
    INIT_RESULT_AND_TEMP;
    IF_NEGATIVE_EXIT(write_spe_start("tr"));
    for (string &text : texts)
    {
        IF_NEGATIVE_EXIT(write_spe("td", text));
    }
    IF_NEGATIVE_EXIT(write_spe_end("tr"));
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
    return write_spe_start("code");
}

ssize_t html_writer::write_code_end()
{
    return write_spe_end("code");
}

ssize_t html_writer::write_xmp_start()
{
    return write_spe_start("xmp");
}

ssize_t html_writer::write_xmp_end()
{
    return write_spe_end("xmp");
}

ssize_t html_writer::write_end()
{
    INIT_RESULT_AND_TEMP;
    const char html_end[] = "</body></html>";
    IF_NEGATIVE_EXIT(send_with_chunk(fd, html_end, sizeof(html_end) - 1, 0));
    IF_NEGATIVE_EXIT(send_chunk_end(fd, 0));
    RETURN_RESULT;
}
