#include "markdown.h"
#include "html_writer.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <filesystem>
#include <sys/socket.h>

using namespace std;
using std::filesystem::exists;

markdown_response::markdown_response(string filename) : filename(move(filename))
{
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

ssize_t deal_with_ul(bool &in_ul, html_writer &writer, vector<string> &texts)
{
    if (in_ul)
    {
        in_ul = false;
        return writer.write_ul(texts);
    }
    return 0;
}

ssize_t deal_with_p(bool &in_p, html_writer &writer)
{
    if (in_p)
    {
        in_p = false;
        return writer.write_p_end();
    }
    return 0;
}

ssize_t markdown_response::send(int fd)
{
    INIT_RESULT_AND_TEMP;
    ifstream ifs(filename);
    html_writer writer(fd);
    IF_NEGATIVE_EXIT(writer.write_head("大作业-Markdown"));
    if (ifs.is_open())
    {
        bool in_code = false;
        bool in_ul = false;
        bool in_p = false;
        vector<string> ul_texts;
        while (!ifs.eof())
        {
            string line;
            getline(ifs, line);
            if (!in_code)
            {
                int index = line.find_first_not_of(' ');
                if (index > 0)
                {
                    line = line.substr(index);
                }
            }
            if (line.length() == 0)
            {
                IF_NEGATIVE_EXIT(deal_with_ul(in_ul, writer, ul_texts));
                IF_NEGATIVE_EXIT(deal_with_p(in_p, writer));
            }
            else if (!in_code && line.length() > 4 && line.substr(0, 4) == "### ")
            {
                IF_NEGATIVE_EXIT(deal_with_ul(in_ul, writer, ul_texts));
                IF_NEGATIVE_EXIT(deal_with_p(in_p, writer));
                IF_NEGATIVE_EXIT(writer.write_h3(deal_with_code(line.substr(4))));
            }
            else if (!in_code && line.length() > 3 && line.substr(0, 3) == "## ")
            {
                IF_NEGATIVE_EXIT(deal_with_ul(in_ul, writer, ul_texts));
                IF_NEGATIVE_EXIT(deal_with_p(in_p, writer));
                IF_NEGATIVE_EXIT(writer.write_h2(deal_with_code(line.substr(3))));
            }
            else if (!in_code && line.length() > 2 && line.substr(0, 2) == "# ")
            {
                IF_NEGATIVE_EXIT(deal_with_ul(in_ul, writer, ul_texts));
                IF_NEGATIVE_EXIT(deal_with_p(in_p, writer));
                IF_NEGATIVE_EXIT(writer.write_h1(deal_with_code(line.substr(2))));
            }
            else if (line.length() > 2 && line.substr(0, 3) == "```")
            {
                IF_NEGATIVE_EXIT(deal_with_p(in_p, writer));
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
                IF_NEGATIVE_EXIT(deal_with_p(in_p, writer));
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
                    IF_NEGATIVE_EXIT(send_with_chunk(fd, line, 0));
                    IF_NEGATIVE_EXIT(send_with_chunk(fd, "\n", 0));
                }
                else
                {
                    if (!in_p)
                    {
                        in_p = true;
                        IF_NEGATIVE_EXIT(send_with_chunk(fd, "<p>", 0));
                    }
                    line = deal_with_code(line);
                    IF_NEGATIVE_EXIT(send_with_chunk(fd, line, 0));
                    IF_NEGATIVE_EXIT(send_with_chunk(fd, " ", 0));
                }
            }
        }
        ifs.close();
    }
    IF_NEGATIVE_EXIT(writer.write_end());
    RETURN_RESULT;
}

void *get_instance_response(const char *command)
{
    if (!command || command[0] == '\0')
    {
        command = "../README.md";
    }
    if (exists(command))
    {
        return new markdown_response(command);
    }
    return nullptr;
}