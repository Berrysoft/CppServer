#include "cpu.h"
#include "proc_cpuinfo.h"
#include "proc_stat.h"
#include "id.h"
#include <sys/socket.h>
#include <string>
#include <sstream>
#include "../../html/html_writer.h"

using namespace std;

void push_linuxcpu(vector<string> &texts, const linuxcpu &cpu)
{
    int *p = (int *)(&cpu);
    for (int i = 0; i < 10; i++)
    {
        texts.push_back(to_string(p[i]));
    }
}

ssize_t cpu_response::send(int fd)
{
    INIT_RESULT_AND_TEMP;
    html_writer writer(fd);
    IF_NEGATIVE_EXIT(writer.write_head("大作业-CPU信息"));
    IF_NEGATIVE_EXIT(writer.write_h1("CPU信息"));

    IF_NEGATIVE_EXIT(writer.write_h2("<a href=\"../file//proc/cpuinfo\">硬件信息</a>"));
    vector<string> texts;
    texts.push_back("名称");
    texts.push_back("核心数");
    texts.push_back("线程数");
    IF_NEGATIVE_EXIT(writer.write_table_start(texts));
    vector<physical_cpu> cpus = read_proc_cpuinfo();
    for (physical_cpu &cpu : cpus)
    {
        texts.clear();
        texts.push_back(cpu.model_name);
        texts.push_back(to_string(cpu.cpu_cores));
        texts.push_back(to_string(cpu.siblings));
        IF_NEGATIVE_EXIT(writer.write_tr(texts));
    }
    IF_NEGATIVE_EXIT(writer.write_table_end());

    IF_NEGATIVE_EXIT(writer.write_h2("<a href=\"../file//proc/stat\">软件信息</a>"));
    texts.clear();
    texts.push_back("序号");
    texts.push_back("user");
    texts.push_back("nice");
    texts.push_back("system");
    texts.push_back("idle");
    texts.push_back("iowait");
    texts.push_back("irq");
    texts.push_back("softirq");
    texts.push_back("steal");
    texts.push_back("guest");
    texts.push_back("guest_nice");
    IF_NEGATIVE_EXIT(writer.write_table_start(texts));
    proc_stat ps = read_proc_stat();
    texts.clear();
    texts.push_back("总CPU");
    push_linuxcpu(texts, ps.total_cpu);
    IF_NEGATIVE_EXIT(writer.write_tr(texts));
    for (size_t i = 0; i < ps.cpu_core.size(); i++)
    {
        texts.clear();
        texts.push_back("CPU " + to_string(i));
        push_linuxcpu(texts, ps.cpu_core[i]);
        IF_NEGATIVE_EXIT(writer.write_tr(texts));
    }
    IF_NEGATIVE_EXIT(writer.write_table_end());
    ostringstream oss;
    oss << "ctxt: " << ps.ctxt << "<br/>" << endl;
    oss << "btime: " << ps.btime << "<br/>" << endl;
    oss << "processes: " << ps.processes << "<br/>" << endl;
    oss << "procs_running: " << ps.procs_running << "<br/>" << endl;
    oss << "procs_blocked: " << ps.procs_blocked << "<br/>" << endl;
    IF_NEGATIVE_EXIT(writer.write_p(oss.str()));

    id i = get_id();
    char buffer[64];
    sprintf(buffer, "<a href=\"../file//proc/%d/status\">进程信息</a>", i.pid);
    IF_NEGATIVE_EXIT(writer.write_h1(buffer));
    oss.str("");
    oss << "UID: " << i.uid << "<br/>" << endl;
    oss << "EUID: " << i.euid << "<br/>" << endl;
    oss << "GID: " << i.gid << "<br/>" << endl;
    oss << "EGID: " << i.egid << "<br/>" << endl;
    oss << "PID: " << i.pid << "<br/>" << endl;
    IF_NEGATIVE_EXIT(writer.write_p(oss.str()));

    IF_NEGATIVE_EXIT(writer.write_end());
    RETURN_RESULT;
}

void *get_instance_response(const char *command)
{
    return new cpu_response();
}
