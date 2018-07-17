#include "markdown.h"
#include "html_writer.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <sys/socket.h>
#include <unistd.h>

using namespace std;

markdown_response::markdown_response(string filename)
{
    this->filename = filename;
}

string deal_with_code(string line)
{
    int index;
    bool in_c = false;
    ostringstream oss;
    if (line[0] == '`')
    {
        oss << "<code>";
        in_c = true;
        line = line.substr(1);
    }
    while ((index = line.find_first_of('`')) > 0)
    {
        string temp = line.substr(0, index);
        line = line.substr(index + 1);
        oss << temp;
        if (in_c)
        {
            oss << "</code>";
            in_c = false;
        }
        else
        {
            oss << "<code>";
            in_c = true;
        }
    }
    oss << line;
    return oss.str();
}

ssize_t markdown_response::send(int fd)
{
    INIT_RESULT_AND_TEMP;
    if (!access(filename.c_str(), 0))
    {
        ifstream ifs(filename);
        html_writer writer(fd);
        IF_NEGATIVE_EXIT(writer.write_head(filename));
        bool in_code = false;
        bool in_ul = false;
        bool in_p = false;
        vector<string> ul_texts;
        while (!ifs.eof())
        {
            string line;
            getline(ifs, line);
            int index = line.find_first_not_of(' ');
            if (index > 0)
            {
                line = line.substr(index);
            }
            if (line.length() == 0)
            {
                if (in_ul)
                {
                    IF_NEGATIVE_EXIT(writer.write_ul(ul_texts));
                    in_ul = false;
                }
                if (in_p)
                {
                    IF_NEGATIVE_EXIT(send_with_chunk(fd, "</p>", 4, 0));
                    in_p = false;
                }
            }
            else if (line.length() > 4 && line.substr(0, 4) == "### ")
            {
                if (in_p)
                {
                    IF_NEGATIVE_EXIT(send_with_chunk(fd, "</p>", 4, 0));
                    in_p = false;
                }
                IF_NEGATIVE_EXIT(writer.write_h3(line.substr(4)));
            }
            else if (line.length() > 3 && line.substr(0, 3) == "## ")
            {
                if (in_p)
                {
                    IF_NEGATIVE_EXIT(send_with_chunk(fd, "</p>", 4, 0));
                    in_p = false;
                }
                IF_NEGATIVE_EXIT(writer.write_h2(line.substr(3)));
            }
            else if (line.length() > 2 && line.substr(0, 2) == "# ")
            {
                if (in_p)
                {
                    IF_NEGATIVE_EXIT(send_with_chunk(fd, "</p>", 4, 0));
                    in_p = false;
                }
                IF_NEGATIVE_EXIT(writer.write_h1(line.substr(2)));
            }
            else if (line.length() > 2 && line.substr(0, 3) == "```")
            {
                if (in_p)
                {
                    IF_NEGATIVE_EXIT(send_with_chunk(fd, "</p>", 4, 0));
                    in_p = false;
                }
                if (in_code)
                {
                    IF_NEGATIVE_EXIT(writer.write_pre_code_end());
                    in_code = false;
                }
                else
                {
                    IF_NEGATIVE_EXIT(writer.write_pre_code_start());
                    in_code = true;
                }
            }
            else if (line.length() > 1 && line.substr(0, 2) == "* ")
            {
                if (in_p)
                {
                    IF_NEGATIVE_EXIT(send_with_chunk(fd, "</p>", 4, 0));
                    in_p = false;
                }
                if (!in_ul)
                {
                    ul_texts.clear();
                    in_ul = true;
                }
                ul_texts.push_back(deal_with_code(line.substr(2)));
            }
            else
            {
                if (in_code)
                {
                    IF_NEGATIVE_EXIT(send_with_chunk(fd, line.c_str(), line.size(), 0));
                    IF_NEGATIVE_EXIT(send_with_chunk(fd, "\n", 1, 0));
                }
                else
                {
                    if (!in_p)
                    {
                        in_p = true;
                        IF_NEGATIVE_EXIT(send_with_chunk(fd, "<p>", 3, 0));
                    }
                    /*int index;
                    bool in_c = false;
                    if (line[0] == '`')
                    {
                        IF_NEGATIVE_EXIT(writer.write_code_start());
                        in_c = true;
                        line = line.substr(1);
                    }
                    while ((index = line.find_first_of('`')) > 0)
                    {
                        string temp = line.substr(0, index);
                        line = line.substr(index + 1);
                        IF_NEGATIVE_EXIT(send_with_chunk(fd, temp.c_str(), temp.size(), 0));
                        if (in_c)
                        {
                            IF_NEGATIVE_EXIT(writer.write_code_end());
                            in_c = false;
                        }
                        else
                        {
                            IF_NEGATIVE_EXIT(writer.write_code_start());
                            in_c = true;
                        }
                    }
                    IF_NEGATIVE_EXIT(send_with_chunk(fd, line.c_str(), line.size(), 0));*/
                    line = deal_with_code(line);
                    IF_NEGATIVE_EXIT(send_with_chunk(fd, line.c_str(), line.length(), 0));
                    IF_NEGATIVE_EXIT(send_with_chunk(fd, " ", 1, 0));
                }
            }
        }
        IF_NEGATIVE_EXIT(writer.write_end());
        ifs.close();
        return result;
    }
    return -1;
}

void *get_instance_response(const char *command)
{
    return new markdown_response("../README.md");
}
