//处理程序运行参数
#pragma once
#include <cstddef>
#include <string>

struct options
{
    bool verbose;
    std::string addr_string;
    int port;
    int amount;
    std::size_t jobs_count;
    int epoll_timeout;
    long interval;
    int clock_timeout;
};

inline options default_options{ false, {}, 3342, 16384, 4, 2000, 60, 2 };

int get_opt(int argc, char* const* argv, options& opt);
