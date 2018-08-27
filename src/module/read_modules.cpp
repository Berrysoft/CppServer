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
            size_t i = line.find_first_not_of(' ');
            if (i != string::npos && i > 0)
                line = line.substr(i);
            if (line.length() == 0)
                continue;
            if (line[0] == '#')
                continue;
            result.push_back(line);
        }
    }
    return result;
}
