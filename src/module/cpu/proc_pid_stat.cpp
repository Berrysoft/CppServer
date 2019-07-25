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
        ifs >> result.pid;
        while (ifs.get() != '(')
            ;
        ostringstream name_stream;
        size_t bra_count = 0;
        while (bra_count || ifs.peek() != ')')
        {
            switch (ifs.peek())
            {
            case '(':
                bra_count++;
                break;
            case ')':
                bra_count--;
                break;
            }
            name_stream << (char)ifs.get();
        }
        result.comm = name_stream.str();
        while (ifs.get() != ' ')
            ;
        ifs >> result.state;
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
    case 'I':
        return "空闲";
    default:
        return {};
    }
}
