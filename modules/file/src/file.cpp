#include <file.h>
#include <filesystem>
#include <fstream>
#include <html/html_writer.h>
#include <linq/string.hpp>
#include <linq/to_container.hpp>
#include <memory>
#include <module/read_modules.h>
#include <sf/sformat.hpp>
#include <sstream>
#include <sys/socket.h>
#include <vector>

using namespace std;
using namespace sf;
using namespace linq;
using std::filesystem::exists;
using std::filesystem::path;

file_response::file_response(init_response_arg* request) : filename(request->command)
{
    string r = request->args;
    if (!r.empty() && r.back() != '&')
    {
        r += '&';
    }
    if (request->method == "POST"s)
    {
        r += request->content;
    }
    auto req_strs = r >> split('&');
    for (auto rs : req_strs)
    {
        vector<string> rls = rs >> split('=') >> to_vector<string>();
        if (rls.size() >= 2)
        {
            m_requires.emplace(rls[0], rls[1]);
        }
        else
        {
            m_requires.emplace(rls[0], string());
        }
    }
    israw = false;
    auto it = m_requires.find("raw");
    if (it != m_requires.end())
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

const char* get_content_type(const string& ex)
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

const char* file_response::type()
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

static unique_ptr<file_response> res_ptr;

int32_t res_init(init_response_arg* arg)
{
    if (arg->version > 1.0)
    {
        res_ptr = make_unique<file_response>(arg);
        return 0;
    }
    return -1;
}

int32_t res_status() { return res_ptr->status(); }

const char* res_type() { return res_ptr->type(); }

ssize_t res_send(int fd) { return res_ptr->send(fd); }

void res_destory() { res_ptr = nullptr; }
