#include <fstream>
#include <mem.h>
#include <sf/format.hpp>
#include <string>

using namespace std;
using namespace sf;

mem_info read_proc_meminfo()
{
    ifstream ifs("/proc/meminfo");
    mem_info result;
    string key;
    int num;
    while (scan(ifs, "{0}{1}\n", key, num))
    {
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
