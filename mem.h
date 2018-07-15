#pragma once

struct mem_info
{
    int total;
    int free;
    int buffers;
    int cached;
    int active;
    int inactive;
};

mem_info read_proc_meminfo();
