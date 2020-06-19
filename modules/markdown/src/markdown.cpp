#include <filesystem>
#include <fstream>
#include <html/html_writer.h>
#include <iterator>
#include <markdown.h>
#include <md4c-html.h>
#include <memory>
#include <sys/socket.h>
#include <vector>

using namespace std;
using std::filesystem::exists;

markdown_response::markdown_response(init_response_arg* arg) : filename(arg->command)
{
    if (filename.empty())
        filename = "README.md";
    file_exists = exists(this->filename);
}

int markdown_response::status()
{
    if (file_exists)
        return 200;
    else
        return 404;
}

ssize_t markdown_response::send(int fd)
{
    INIT_RESULT_AND_TEMP;
    ifstream ifs(filename);
    html_writer writer(fd);
    IF_NEGATIVE_EXIT(writer.write_head("大作业-Markdown"));
    if (file_exists)
    {
        string buffer(istreambuf_iterator<char>{ ifs }, istreambuf_iterator<char>{});
        struct user_data_pack
        {
            int fd;
        } user_data = { fd };
        IF_NEGATIVE_EXIT(md_html(
            buffer.c_str(), (MD_SIZE)buffer.size(), [](const MD_CHAR* text, MD_SIZE size, void* data) {
                int fd = ((user_data_pack*)data)->fd;
                send_with_chunk(fd, text, (size_t)size, 0);
            },
            (void*)&user_data, 0, 0));
    }
    else
    {
        IF_NEGATIVE_EXIT(writer.write_h1("找不到文件"));
        IF_NEGATIVE_EXIT(writer.write_p("您请求的文件找不到，请确认输入是否正确。"));
    }
    IF_NEGATIVE_EXIT(writer.write_end());
    RETURN_RESULT;
}

static unique_ptr<markdown_response> res_ptr;

int32_t res_init(init_response_arg* arg)
{
    if (arg->version > 1.0)
    {
        res_ptr = make_unique<markdown_response>(arg);
        return 0;
    }
    return -1;
}

int32_t res_status() { return res_ptr->status(); }

ssize_t res_send(int fd) { return res_ptr->send(fd); }

void res_destory() { res_ptr = nullptr; }
