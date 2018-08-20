#include "read_modules.h"
#include <fstream>
#include <unistd.h>

using namespace std;

vector<string> read_modules_file()
{
    vector<string> result;
    if (access("modules", 0))
    {
        return result;
    }
    ifstream ifs("modules");
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
    return result;
}
