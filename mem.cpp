#include "mem.h"
#include <fstream>
#include <string>
#include <sstream>

using std::ifstream;
using std::istringstream;
using std::string;

mem_info read_proc_meminfo()
{
    ifstream ifs("/proc/meminfo");
    mem_info result;
    while (!ifs.eof())
    {
        string line;
        getline(ifs, line);
        istringstream iss(line);
        string key;
        int num;
        iss >> key >> num;
        if (key == "MemTotal:")
        {
            result.total = num;
        }
        else if (key == "MemFree:")
        {
            result.free = num;
        }
        else if (key == "Buffers:")
        {
            result.buffers = num;
        }
        else if (key == "Cached:")
        {
            result.cached = num;
        }
        else if (key == "Active:")
        {
            result.active = num;
        }
        else if (key == "Inactive:")
        {
            result.inactive = num;
        }
    }
    return result;
}
