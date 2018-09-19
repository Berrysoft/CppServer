#include <fstream>
#include <module/cpu/proc_pid_stat.h>
#include <sf/sformat.hpp>

using namespace std;
using namespace sf;

process_stat read_proc_stat(int pid)
{
    ifstream ifs(sprint("/proc/{0}/stat", pid));
    if (ifs.is_open())
    {
        process_stat result;
        string name;
        scan(ifs, "{0}{1}{2}", result.pid, name, result.state);
        if (name.front() == '(')
            name = name.substr(1);
        if (name.back() == ')')
            name.pop_back();
        result.comm = name;
        return result;
    }
    return {};
}

string get_state_str(char state)
{
    switch (state)
    {
    case 'R':
        return "运行中";
    case 'S':
        return "睡眠";
    case 'D':
        return "休眠";
    case 'Z':
        return "僵尸进程";
    case 'T':
        return "停止";
    case 't':
        return "停止跟踪";
    case 'X':
        return "死亡";
    default:
        return {};
    }
}
