#pragma once
#include <string>
#include <vector>

struct physical_cpu
{
    std::string model_name;
    int cpu_cores;
    int siblings;
};

std::vector<physical_cpu> read_proc_cpuinfo();