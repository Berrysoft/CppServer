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

ssize_t markdown_response::send(int fd)
{
    ssize_t result = 0;
    if (!access(filename.c_str(), 0))
    {
        ifstream ifs(filename);
        html_writer writer(fd);
        result += writer.write_head(filename);
        bool in_code = false;
        bool in_ul = false;
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
                    result += writer.write_ul(ul_texts);
                    in_ul = false;
                }
                result += ::send(fd, "<br/>", 5, 0);
            }
            else if (line.length() > 4 && line.substr(0, 4) == "### ")
            {
                result += writer.write_h3(line.substr(4));
            }
            else if (line.length() > 3 && line.substr(0, 3) == "## ")
            {
                result += writer.write_h2(line.substr(3));
            }
            else if (line.length() > 2 && line.substr(0, 2) == "# ")
            {
                result += writer.write_h1(line.substr(2));
            }
            else if (line.length() > 2 && line.substr(0, 3) == "```")
            {
                if (in_code)
                {
                    result += writer.write_pre_code_end();
                    in_code = false;
                }
                else
                {
                    result += writer.write_pre_code_start();
                    in_code = true;
                }
            }
            else if (line.length() > 1 && line.substr(0, 2) == "* ")
            {
                if (!in_ul)
                {
                    ul_texts.clear();
                    in_ul = true;
                }
                ul_texts.push_back(line.substr(2));
            }
            else
            {
                if (in_code)
                {
                    result += ::send(fd, line.c_str(), line.size(), 0);
                }
                else
                {
                    result += writer.write_p(line);
                }
                result += ::send(fd, "\n", 1, 0);
            }
        }
        result += writer.write_end();
        return result;
    }
    return -1;
}

void *get_instance_response(const char *command)
{
    return new markdown_response("README.md");
}
