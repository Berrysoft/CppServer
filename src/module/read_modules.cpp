#include "read_modules.h"
#include <filesystem>
#include <fstream>

using namespace std;
using std::filesystem::exists;

const char modules_file[] = "modules";

vector<string> read_modules_file()
{
    vector<string> result;
    if (exists(modules_file))
    {
        ifstream ifs(modules_file);
        while (!ifs.eof())
        {
            string line;
            getline(ifs, line);
            if (line.back() == '\r')
                line.pop_back();
            size_t i = line.find_first_not_of(" \t");
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
