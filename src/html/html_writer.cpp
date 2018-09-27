#include <fstream>
#include <html/html_writer.h>
#include <sf/sformat.hpp>
#include <string_view>
#include <sys/socket.h>

using namespace std;
using namespace sf;

ssize_t send_with_chunk(int fd, const char* buffer, size_t length, int flag)
{
    INIT_RESULT_AND_TEMP;
    string buf = sprint("{0:X}\r\n{1}\r\n", length, sf::string_view(buffer, length));
    IF_NEGATIVE_EXIT(send(fd, buf.c_str(), buf.length(), flag));
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
    const char head_mid[] = "</title><style>";
    const char head_end[] = "</style></head><body>";
    IF_NEGATIVE_EXIT(send_with_chunk(fd, head_start, 0));
    IF_NEGATIVE_EXIT(send_with_chunk(fd, title, 0));
    IF_NEGATIVE_EXIT(send_with_chunk(fd, head_mid, 0));
    ifstream fcss("style.css");
    if (fcss.is_open())
    {
        char buffer[4096];
        while (!fcss.eof())
        {
            fcss.read(buffer, sizeof(buffer));
            IF_NEGATIVE_EXIT(send_with_chunk(fd, buffer, fcss.gcount(), 0));
        }
        fcss.close();
    }
    IF_NEGATIVE_EXIT(send_with_chunk(fd, head_end, 0));
    RETURN_RESULT;
}

ssize_t html_writer::write_spe(string spe, string text)
{
    string s = sprint("<{0}>{1}</{0}>", spe, text);
    return send_with_chunk(fd, s, 0);
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
    string t = sprint("<{0}>", spe);
    return send_with_chunk(fd, t, 0);
}

ssize_t html_writer::write_spe_end(string spe)
{
    string t = sprint("</{0}>", spe);
    return send_with_chunk(fd, t, 0);
}

ssize_t html_writer::write_ul(vector<string> texts)
{
    INIT_RESULT_AND_TEMP;
    IF_NEGATIVE_EXIT(write_spe_start("ul"));
    for (string& text : texts)
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
    IF_NEGATIVE_EXIT(send_with_chunk(fd, table_start, 0));
    for (string& text : texts)
    {
        IF_NEGATIVE_EXIT(write_spe("th", text));
    }
    IF_NEGATIVE_EXIT(send_with_chunk(fd, thead_end, 0));
    RETURN_RESULT;
}

ssize_t html_writer::write_tr(vector<string> texts)
{
    INIT_RESULT_AND_TEMP;
    IF_NEGATIVE_EXIT(write_spe_start("tr"));
    for (string& text : texts)
    {
        IF_NEGATIVE_EXIT(write_spe("td", text));
    }
    IF_NEGATIVE_EXIT(write_spe_end("tr"));
    RETURN_RESULT;
}

ssize_t html_writer::write_table_end()
{
    const char table_end[] = "</tbody></table>";
    return send_with_chunk(fd, table_end, 0);
}

ssize_t html_writer::write_pre_code_start()
{
    const char code_start[] = "<pre><code>";
    return send_with_chunk(fd, code_start, 0);
}

ssize_t html_writer::write_pre_code_end()
{
    const char code_end[] = "</code></pre>";
    return send_with_chunk(fd, code_end, 0);
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
    IF_NEGATIVE_EXIT(send_with_chunk(fd, html_end, 0));
    IF_NEGATIVE_EXIT(send_chunk_end(fd, 0));
    RETURN_RESULT;
}
