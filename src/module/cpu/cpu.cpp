#include <html/html_writer.h>
#include <module/cpu/cpu.h>
#include <module/cpu/id.h>
#include <module/cpu/proc_cpuinfo.h>
#include <module/cpu/proc_stat.h>
#include <sf/sformat.hpp>
#include <string>
#include <sys/socket.h>

using namespace std;
using namespace sf;

void push_linuxcpu(vector<string>& texts, const linuxcpu& cpu)
{
    int* p = (int*)(&cpu);
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
    for (physical_cpu& cpu : cpus)
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
        texts.push_back(sprint("CPU {0}"sv, i));
        push_linuxcpu(texts, ps.cpu_core[i]);
        IF_NEGATIVE_EXIT(writer.write_tr(texts));
    }
    IF_NEGATIVE_EXIT(writer.write_table_end());
    IF_NEGATIVE_EXIT(writer.write_p(
        sprint("ctxt: {0}<br/>\n"
               "btime: {1}<br/>\n"
               "processes: {2}<br/>\n"
               "procs_running: {3}<br/>\n"
               "procs_blocked: {4}<br/>\n"sv,
               ps.ctxt, ps.btime, ps.processes, ps.procs_running, ps.procs_blocked)));

    id i = get_id();
    IF_NEGATIVE_EXIT(writer.write_h1(
        sprint("<a href=\"../file//proc/{0}/status\">进程信息</a>"sv, i.pid)));
    IF_NEGATIVE_EXIT(writer.write_p(
        sprint("UID: {0}<br/>\n"
               "EUID: {1}<br/>"
               "GID: {2}<br/>\n"
               "EGID: {3}<br/>\n"
               "PID: {4}<br/>\n"sv,
               i.uid, i.euid, i.gid, i.egid, i.pid)));

    IF_NEGATIVE_EXIT(writer.write_end());
    RETURN_RESULT;
}

void* get_instance_response(void* request)
{
    const http_request& req = *(const http_request*)request;
    if (req.version() != HTTP_1_0)
    {
        return new cpu_response(req);
    }
    return nullptr;
}
