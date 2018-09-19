//通过打开/proc/<pid>/stat生成进程信息
#pragma once
#include <string>

struct process_stat
{
    int pid;
    std::string comm;
    char state;
};

process_stat read_proc_stat(int pid);

std::string get_state_str(char state);
