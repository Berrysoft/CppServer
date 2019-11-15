#include <filesystem>
#include <html/html_writer.h>
#include <linq/aggregate.hpp>
#include <linq/query.hpp>
#include <linq/to_container.hpp>
#include <module/cpu/proc_cpuinfo.h>
#include <module/cpu/proc_pid_stat.h>
#include <module/cpu/proc_stat.h>
#include <module/response.h>
#include <sf/sformat.hpp>
#include <string>
#include <sys/socket.h>
#include <vector>

using namespace std;
using namespace std::filesystem;
using namespace sf;
using namespace linq;

void push_linuxcpu(vector<string>& texts, const linuxcpu& cpu)
{
    int* p = (int*)(&cpu);
    get_enumerable(move(p), p + 10) >>
        select([](int& i) { return to_string(i); }) >>
        for_each([&texts](string s) { texts.push_back(move(s)); });
}

int32_t res_init(init_response_arg* arg)
{
    if (arg->version > 1.0)
    {
        return 0;
    }
    return -1;
}

int32_t res_destory() { return 0; }

ssize_t res_send(int fd)
{
    INIT_RESULT_AND_TEMP;
    html_writer writer(fd);
    IF_NEGATIVE_EXIT(writer.write_head("大作业-CPU信息"));
    IF_NEGATIVE_EXIT(writer.write_h1("CPU信息"));

    IF_NEGATIVE_EXIT(writer.write_h2("<a href=\"../file//proc/cpuinfo\">硬件信息</a>"));
    vector<string> texts{ "名称", "核心数", "线程数" };
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
    texts = vector<string>{ "序号", "user", "nice", "system", "idle", "iowait", "irq", "softirq", "steal", "guest", "guest_nice" };
    IF_NEGATIVE_EXIT(writer.write_table_start(texts));
    proc_stat ps = read_proc_stat();
    texts.clear();
    texts.push_back("总CPU");
    push_linuxcpu(texts, ps.total_cpu);
    IF_NEGATIVE_EXIT(writer.write_tr(texts));
    for (size_t i = 0; i < ps.cpu_core.size(); i++)
    {
        texts.clear();
        texts.push_back(sprint("CPU {0}", i));
        push_linuxcpu(texts, ps.cpu_core[i]);
        IF_NEGATIVE_EXIT(writer.write_tr(texts));
    }
    IF_NEGATIVE_EXIT(writer.write_table_end());
    IF_NEGATIVE_EXIT(writer.write_p(
        sprint("ctxt: {0}<br/>\n"
               "btime: {1}<br/>\n"
               "processes: {2}<br/>\n"
               "procs_running: {3}<br/>\n"
               "procs_blocked: {4}<br/>\n",
               ps.ctxt, ps.btime, ps.processes, ps.procs_running, ps.procs_blocked)));

    IF_NEGATIVE_EXIT(writer.write_h1("<a href=\"../file//proc/self/status\">进程信息</a>"));
    texts.clear();
    texts.push_back("PID");
    texts.push_back("名称");
    texts.push_back("状态");
    IF_NEGATIVE_EXIT(writer.write_table_start(texts));
    for (auto& d : directory_iterator("/proc"))
    {
        if (d.is_directory())
        {
            const path& name = d;
            int pid = (int)strtol(name.filename().c_str(), nullptr, 10);
            if (pid > 0)
            {
                texts.clear();
                process_stat pstat = read_proc_stat(pid);
                texts.push_back(sprint("<a href=\"../file//proc/{0}/status\">{0}</a>", pstat.pid));
                texts.push_back(pstat.comm);
                texts.push_back(get_state_str(pstat.state));
                IF_NEGATIVE_EXIT(writer.write_tr(texts));
            }
        }
    }
    IF_NEGATIVE_EXIT(writer.write_table_end());

    IF_NEGATIVE_EXIT(writer.write_end());
    RETURN_RESULT;
}
