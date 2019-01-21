#include <fstream>
#include <linq/string.hpp>
#include <module/cpu/proc_cpuinfo.h>
#include <sf/sformat.hpp>
#include <string>

using namespace std;
using namespace sf;
using namespace linq;

string get_first_info(istream& is, string head)
{
    string inf;
    string line;
    while (getline(is, line))
    {
        if (line >> starts_with<char>(head))
        {
            sscan(line, ": {0}", inf);
            return inf;
        }
    }
    return string();
}

vector<physical_cpu> read_proc_cpuinfo()
{
    ifstream ifs("/proc/cpuinfo");
    int pid = -1;
    string p;
    string line;
    string mname;
    vector<physical_cpu> result;
    while (!ifs.eof())
    {
        p = get_first_info(ifs, "processor");
        if (p.length() == 0)
            break;
        mname = get_first_info(ifs, "model name");
        int tpid = stoi(get_first_info(ifs, "physical id"));
        int sibs = stoi(get_first_info(ifs, "siblings"));
        int cores = stoi(get_first_info(ifs, "cpu cores"));
        if (tpid > pid)
        {
            physical_cpu pcpu = { mname, cores, sibs };
            result.push_back(pcpu);
            pid++;
        }
    }
    return result;
}
