#include "file.h"
#include "../../html/html_writer.h"
#include "../read_modules.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <sys/socket.h>
#include <vector>

using namespace std;
using std::filesystem::exists;

file_response::file_response(string filename) : filename(move(filename))
{
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
            ostringstream oss;
            oss << "<a href=\"../" << key << "/\">" << text << "</a>";
            texts.push_back(oss.str());
        }
        IF_NEGATIVE_EXIT(writer.write_ul(texts));
        IF_NEGATIVE_EXIT(writer.write_end());
    }
    else
    {
        IF_NEGATIVE_EXIT(writer.write_head("大作业-文件"));
        IF_NEGATIVE_EXIT(writer.write_h1(filename));
        IF_NEGATIVE_EXIT(writer.write_pre_code_start());
        IF_NEGATIVE_EXIT(writer.write_xmp_start());
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
        IF_NEGATIVE_EXIT(writer.write_xmp_end());
        IF_NEGATIVE_EXIT(writer.write_pre_code_end());
        IF_NEGATIVE_EXIT(writer.write_end());
    }
    RETURN_RESULT;
}

void* get_instance_response(const char* command)
{
    if (command && (command[0] == '\0' || exists(command)))
    {
        return new file_response(command);
    }
    return nullptr;
}
