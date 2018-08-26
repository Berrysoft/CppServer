#include "raw.h"
#include <sys/socket.h>
#include <fstream>
#include <filesystem>
#include "../../html/html_writer.h"

using namespace std;
using std::filesystem::exists;
using std::filesystem::path;

raw_response::raw_response(string filename) : filename(move(filename))
{
}

string get_content_type(const string &ex)
{
    if (ex == "htm" || ex == "html")
    {
        return "text/html";
    }
    else if (ex == "css")
    {
        return "text/css";
    }
    else if (ex == "xml")
    {
        return "application/xml";
    }
    else if (ex == "pdf")
    {
        return "application/pdf";
    }
    else if (ex == "txt")
    {
        return "text/plain";
    }
    else
    {
        return "application/octet-stream";
    }
}

string raw_response::type()
{
    if (filename.empty())
    {
        return "text/html";
    }
    else
    {
        string ex = path(filename).extension();
        if (!ex.empty() && ex.front() == '.')
        {
            ex.erase(ex.begin());
        }
        if (ex.empty())
        {
            return "text/plain";
        }
        return get_content_type(ex);
    }
}

ssize_t raw_response::send(int fd)
{
    INIT_RESULT_AND_TEMP;
    if (filename.empty())
    {
        html_writer writer(fd);
        IF_NEGATIVE_EXIT(writer.write_head("大作业-源文件"));
        IF_NEGATIVE_EXIT(writer.write_h1("大作业-源文件"));
        IF_NEGATIVE_EXIT(writer.write_p("想要查看源文件，在请求<code>/raw/</code>后面加上文件路径。"));
        IF_NEGATIVE_EXIT(writer.write_end());
    }
    else
    {
        ifstream ifs(filename);
        if (ifs.is_open())
        {
            char buffer[4096];
            while (!ifs.eof())
            {
                ifs.read(buffer, sizeof(buffer));
                IF_NEGATIVE_EXIT(send_with_chunk(fd, buffer, ifs.gcount(), 0));
            }
            IF_NEGATIVE_EXIT(send_chunk_end(fd, 0));
            ifs.close();
        }
    }
    RETURN_RESULT;
}

void *get_instance_response(const char *command)
{
    if (command && (command[0] == '\0' || exists(command)))
    {
        return new raw_response(command);
    }
    return nullptr;
}
