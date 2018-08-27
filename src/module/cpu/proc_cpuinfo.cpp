#include "proc_cpuinfo.h"
#include <cstdlib>
#include <fstream>
#include <ios>

using namespace std;

string get_first_info(istream& is, string head)
{
    while (true)
    {
        if (is.eof())
        {
            return string();
        }
        string temp;
        getline(is, temp);
        if (temp.substr(0, head.length()) == head)
        {
            size_t i = temp.find_first_of(':');
            return temp.substr(i + 2);
        }
    }
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
            physical_cpu pcpu = {mname, cores, sibs};
            result.push_back(pcpu);
            pid++;
        }
    }
    return result;
}
