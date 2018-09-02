//处理程序运行参数
#pragma once
#include <cstddef>

struct options
{
    bool verbose;
    char* addr_string;
    int port;
    int amount;
    std::size_t jobs_count;
    int epoll_timeout;
    long interval;
    int clock_timeout;
};

constexpr options default_options = { false, nullptr, 3342, 16384, 4, 2000, 60, 2 };

int get_opt(int argc, char** argv, options& opt);
