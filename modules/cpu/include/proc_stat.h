//生成CPU使用信息。
#pragma once
#include <vector>

struct linuxcpu
{
    int user;
    int nice;
    int system;
    int idle;
    int iowait;
    int irq;
    int softirq;
    int steal;
    int guest;
    int guest_nice;
};

struct proc_stat
{
    linuxcpu total_cpu;
    std::vector<linuxcpu> cpu_core;
    int ctxt;
    int btime;
    int processes;
    int procs_running;
    int procs_blocked;
};

proc_stat read_proc_stat();