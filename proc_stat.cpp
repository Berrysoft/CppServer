#include "proc_stat.h"
#include <ios>
#include <fstream>
#include <string>

using std::istream;
using std::ifstream;
using std::string;
using std::getline;

void read_linuxcpu(istream& is, linuxcpu& lcpu)
{
    is >> lcpu.user >> lcpu.nice >> lcpu.system >> lcpu.idle >> lcpu.iowait >> lcpu.irq >> lcpu.softirq >> lcpu.steal >> lcpu.guest >> lcpu.guest_nice;
}

bool read_with_head(istream& is, const char* head, int& value)
{
    string t;
    is >> t;
    if (t == head)
    {
        is >> value;
        return true;
    }
    return false;
}

proc_stat read_proc_stat()
{
    ifstream ifs("/proc/stat");
    proc_stat result;
    string head;
    ifs >> head;
    if (head == "cpu")
    {
        read_linuxcpu(ifs, result.total_cpu);
        while (true)
        {
            ifs >> head;
            if (head.substr(0, 3) == "cpu")
            {
                linuxcpu t;
                read_linuxcpu(ifs, t);
                result.cpu_core.push_back(t);
            }
            else
            {
                break;
            }
        }
        getline(ifs, head);
        if (read_with_head(ifs, "ctxt", result.ctxt) &&
            read_with_head(ifs, "btime", result.btime) &&
            read_with_head(ifs, "processes", result.processes) &&
            read_with_head(ifs, "procs_running", result.procs_running) &&
            read_with_head(ifs, "procs_blocked", result.procs_blocked))
        {
            ;
        }
    }
    ifs.close();
    return result;
}
