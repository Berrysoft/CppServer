#include <module/read_modules.h>
#include <fstream>

using namespace std;

const char modules_file[] = "modules";

vector<string> read_modules_file()
{
    vector<string> result;
    ifstream ifs(modules_file);
    if (ifs.is_open())
    {
        while (!ifs.eof())
        {
            string line;
            getline(ifs, line);
            size_t i = line.find_first_not_of(" \t\r");
            if (i != string::npos)
                line = line.substr(i);
            else
                continue;
            if (line.empty())
                continue;
            if (line.front() == '#')
                continue;
            result.push_back(line);
        }
    }
    return result;
}
