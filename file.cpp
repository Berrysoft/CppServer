#include "file.h"
#include <cstdio>
#include <cstring>
#include <sys/socket.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <unistd.h>
#include "html_writer.h"
#include "read_modules.h"

using namespace std;

file_response::file_response(const char *filename)
{
    this->filename = string(filename);
}

ssize_t file_response::send(int fd)
{
    ssize_t result = 0;
    html_writer writer(fd);
    if (filename == "index.html")
    {
        result += writer.write_head("大作业-主页");
        result += writer.write_h1("大作业-主页");
        result += writer.write_p("欢迎光临！");
        result += writer.write_h2("动态加载功能");
        vector<string> lines = read_modules_file();
        vector<string> texts;
        for (string &line : lines)
        {
            istringstream iss(line);
            string key, module_name, text;
            iss >> key >> module_name >> text;
            ostringstream oss;
            oss << "<a href=\"../" << key << "/\">" << text << "</a>";
            texts.push_back(oss.str());
        }
        result += writer.write_ul(texts);
        result += writer.write_end();
    }
    else if (filename == "error.html")
    {
        result += writer.write_head("大作业-错误");
        result += writer.write_h1("出错啦！");
        result += writer.write_p("我们找不到请求的文件或者请求无效，请返回到上一页。");
        result += writer.write_end();
    }
    else if (filename.length() == 0)
    {
        result += writer.write_head("大作业-文件");
        result += writer.write_h1("查看文件");
        result += writer.write_p("想要查看文件，直接在地址栏输入即可，不需要加上/file/请求。");
        result += writer.write_end();
    }
    else
    {
        result += writer.write_head("大作业-文件");
        result += writer.write_h1(filename);
        result += writer.write_code_start();
        result += writer.write_xmp_start();
        FILE *furl = fopen(filename.c_str(), "r");
        if (furl)
        {
            std::size_t len;
            char buffer[4096];
            memset(buffer, 0, sizeof(buffer));
            while (len = fread(buffer, sizeof(char), sizeof(buffer), furl))
            {
                result += send_with_chunk(fd, buffer, len, 0);
            }
            fclose(furl);
        }
        result += writer.write_xmp_end();
        result += writer.write_code_end();
        result += writer.write_end();
    }
    return result;
}

void *get_instance_response(const char *command)
{
    return new file_response(command);
}
