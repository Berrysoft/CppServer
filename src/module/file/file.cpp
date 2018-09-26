#include <filesystem>
#include <fstream>
#include <html/html_writer.h>
#include <module/file/file.h>
#include <module/read_modules.h>
#include <sf/sformat.hpp>
#include <sstream>
#include <sys/socket.h>
#include <vector>

using namespace std;
using namespace sf;
using std::filesystem::exists;
using std::filesystem::path;

vector<string> split(const string& str, char c)
{
    vector<string> result;
    size_t index = 0;
    while (index != string::npos && index < str.length())
    {
        size_t i = str.find(c, index);
        if (i != string::npos)
        {
            result.push_back(str.substr(index, i - index));
            index = i + 1;
        }
        else
        {
            result.push_back(str.substr(index));
            break;
        }
    }
    return result;
}

file_response::file_response(const http_request& request, string filename) : response(request), filename(move(filename))
{
    string r = request.split_url().args;
    if (!r.empty() && r.back() != '&')
    {
        r += '&';
    }
    if (request.method() == "POST")
    {
        r += request.content();
    }
    vector<string> req_strs = split(r, '&');
    for (string& rs : req_strs)
    {
        vector<string> rls = split(rs, '=');
        if (rls.size() >= 2)
        {
            requires.emplace(rls[0], rls[1]);
        }
        else
        {
            requires.emplace(rls[0], string());
        }
    }
    israw = false;
    auto it = requires.find("raw");
    if (it != requires.end())
    {
        string raw_str = it->second;
        if (!raw_str.empty())
        {
            israw = stoi(raw_str);
        }
    }
    file_exists = this->filename.empty() || exists(this->filename);
}

int file_response::status()
{
    if (file_exists)
        return 200;
    else
        return 404;
}

string get_content_type(const string& ex)
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

string file_response::type()
{
    if (israw)
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
    else
    {
        return "text/html";
    }
}

ssize_t file_response::send(int fd)
{
    INIT_RESULT_AND_TEMP;
    html_writer writer(fd);
    if (filename.empty())
    {
        IF_NEGATIVE_EXIT(writer.write_head("大作业-主页"));
        IF_NEGATIVE_EXIT(writer.write_h1("大作业-主页"));
        IF_NEGATIVE_EXIT(writer.write_p("欢迎光临！想要查看文件，在请求<code>/file/</code>后面加上文件路径。"));
        IF_NEGATIVE_EXIT(writer.write_h2("动态加载功能"));
        vector<string> lines = read_modules_file();
        vector<string> texts;
        for (string& line : lines)
        {
            istringstream iss(line);
            string key, module_name, text;
            iss >> key >> module_name >> text;
            texts.push_back(sprint("<a href=\"../{0}/\">{1}</a>", key, text));
        }
        IF_NEGATIVE_EXIT(writer.write_ul(texts));
        IF_NEGATIVE_EXIT(writer.write_end());
    }
    else if (file_exists)
    {
        if (!israw)
        {
            IF_NEGATIVE_EXIT(writer.write_head("大作业-文件"));
            IF_NEGATIVE_EXIT(writer.write_h1(filename));
            IF_NEGATIVE_EXIT(writer.write_pre_code_start());
            IF_NEGATIVE_EXIT(writer.write_xmp_start());
        }
        ifstream furl(filename);
        if (furl.is_open())
        {
            char buffer[4096];
            while (!furl.eof())
            {
                furl.read(buffer, sizeof(buffer));
                IF_NEGATIVE_EXIT(send_with_chunk(fd, buffer, furl.gcount(), 0));
            }
            furl.close();
        }
        if (!israw)
        {
            IF_NEGATIVE_EXIT(writer.write_xmp_end());
            IF_NEGATIVE_EXIT(writer.write_pre_code_end());
            IF_NEGATIVE_EXIT(writer.write_end());
        }
        else
        {
            IF_NEGATIVE_EXIT(send_chunk_end(fd, 0));
        }
    }
    else
    {
        IF_NEGATIVE_EXIT(writer.write_head("大作业-文件"));
        IF_NEGATIVE_EXIT(writer.write_h1("找不到文件"));
        IF_NEGATIVE_EXIT(writer.write_p("您请求的文件找不到，请确认输入是否正确。"));
        IF_NEGATIVE_EXIT(writer.write_end());
    }
    RETURN_RESULT;
}

void* get_instance_response(void* request)
{
    const http_request& req = *(const http_request*)request;
    if (req.version() > 1.0)
    {
        string command = req.split_url().command;
        return new file_response(req, command);
    }
    return nullptr;
}
