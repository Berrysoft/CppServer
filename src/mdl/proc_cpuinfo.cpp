#include "proc_cpuinfo.h"
#include <ios>
#include <fstream>
#include <cstdlib>

using std::size_t;
using std::vector;
using std::string;
using std::istream;
using std::ifstream;
using std::getline;
using std::atoi;

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
        int tpid = atoi(get_first_info(ifs, "physical id").c_str());
        int sibs = atoi(get_first_info(ifs, "siblings").c_str());
        int cores = atoi(get_first_info(ifs, "cpu cores").c_str());
        if (tpid > pid)
        {
            physical_cpu pcpu = { mname,cores,sibs };
            result.push_back(pcpu);
            pid++;
        }
    }
    return result;
}
