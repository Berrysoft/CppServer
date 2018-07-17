#include "file.h"
#include <cstdio>
#include <cstring>
#include <sys/socket.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <unistd.h>
#include "html_writer.h"
#include "../read_modules.h"

using namespace std;

file_response::file_response(const char *filename)
{
    this->filename = string(filename);
}

ssize_t file_response::send(int fd)
{
    INIT_RESULT_AND_TEMP;
    html_writer writer(fd);
    if (filename == "index.html")
    {
        IF_NEGATIVE_EXIT(writer.write_head("大作业-主页"));
        IF_NEGATIVE_EXIT(writer.write_h1("大作业-主页"));
        IF_NEGATIVE_EXIT(writer.write_p("欢迎光临！"));
        IF_NEGATIVE_EXIT(writer.write_h2("动态加载功能"));
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
        IF_NEGATIVE_EXIT(writer.write_ul(texts));
        IF_NEGATIVE_EXIT(writer.write_end());
    }
    else if (filename == "error.html")
    {
        IF_NEGATIVE_EXIT(writer.write_head("大作业-错误"));
        IF_NEGATIVE_EXIT(writer.write_h1("出错啦！"));
        IF_NEGATIVE_EXIT(writer.write_p("我们找不到请求的文件或者请求无效，请返回到上一页。"));
        IF_NEGATIVE_EXIT(writer.write_end());
    }
    else if (filename.length() == 0)
    {
        IF_NEGATIVE_EXIT(writer.write_head("大作业-文件"));
        IF_NEGATIVE_EXIT(writer.write_h1("查看文件"));
        IF_NEGATIVE_EXIT(writer.write_p("想要查看文件，直接在地址栏输入即可，不需要加上/file/请求。"));
        IF_NEGATIVE_EXIT(writer.write_end());
    }
    else
    {
        IF_NEGATIVE_EXIT(writer.write_head("大作业-文件"));
        IF_NEGATIVE_EXIT(writer.write_h1(filename));
        IF_NEGATIVE_EXIT(writer.write_pre_code_start());
        IF_NEGATIVE_EXIT(writer.write_xmp_start());
        FILE *furl = fopen(filename.c_str(), "r");
        if (furl)
        {
            std::size_t len;
            char buffer[4096];
            memset(buffer, 0, sizeof(buffer));
            while (len = fread(buffer, sizeof(char), sizeof(buffer), furl))
            {
                IF_NEGATIVE_EXIT(send_with_chunk(fd, buffer, len, 0));
            }
            fclose(furl);
        }
        IF_NEGATIVE_EXIT(writer.write_xmp_end());
        IF_NEGATIVE_EXIT(writer.write_pre_code_end());
        IF_NEGATIVE_EXIT(writer.write_end());
    }
    RETURN_RESULT;
}

void *get_instance_response(const char *command)
{
    return new file_response(command);
}
