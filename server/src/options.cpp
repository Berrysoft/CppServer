#include <cstdlib>
#include <getopt.h>
#include <options.h>
#include <sf/color.hpp>
#include <sf/format.hpp>

using namespace std;
using namespace sf;

const char help_opt[] = "help";
const char verbose_opt[] = "verbose";
const char addr_opt[] = "address";
const char port_opt[] = "port";
const char count_opt[] = "count";
const char threads_opt[] = "threads";
const char etime_opt[] = "epoll-timeout";
const char cinterval_opt[] = "clock-interval";
const char ctime_opt[] = "clock-timeout";

const char s_opts[] = "hva:p:c:t:e:i:o:";
const option l_opts[] = {
    { help_opt, no_argument, nullptr, 'h' },
    { verbose_opt, no_argument, nullptr, 'v' },
    { addr_opt, required_argument, nullptr, 'a' },
    { port_opt, required_argument, nullptr, 'p' },
    { count_opt, required_argument, nullptr, 'c' },
    { threads_opt, required_argument, nullptr, 't' },
    { etime_opt, required_argument, nullptr, 'e' },
    { cinterval_opt, required_argument, nullptr, 'i' },
    { ctime_opt, required_argument, nullptr, 'o' },
    { nullptr, 0, nullptr, 0 }
};

void print_help(char* const name)
{
    print("Berrysoft.Linux.Cpp.Server\n");
    print("网站在Edge、IE、Chrome、Firefox、Safari(iPhone)下测试通过。\n");
    print("请使用支持chunked的浏览器打开网站。\n");
    print("用法：{0} [-hv]|[-apcteio 参数]\n", make_color_arg(name, bright_blue, user_default, bold));
    print("选项：\n");
    print("-h --{0:l16}获取帮助\n", help_opt);
    print("-v --{0:l16}显示详细信息\n", verbose_opt);
    print("-a --{0:l16}设置监听地址，默认为所有IP\n", addr_opt);
    print("-p --{0:l16}设置监听端口，默认为3342\n", port_opt);
    print("-c --{0:l16}设置监听数，默认为16384\n", count_opt);
    print("-t --{0:l16}设置线程数，默认为4\n", threads_opt);
    print("-e --{0:l16}设置Epoll的等待时间，默认为2000(ms)\n", etime_opt);
    print("-i --{0:l16}设置时钟间隔，默认为60(s)\n", cinterval_opt);
    print("-o --{0:l16}设置时钟等待循环数，默认为2（个）\n", ctime_opt);
}

int get_opt(int argc, char* const* argv, options& opt)
{
    int optc;
    while ((optc = getopt_long(argc, argv, s_opts, l_opts, nullptr)) >= 0)
    {
        switch (optc)
        {
        case 'v':
            opt.verbose = true;
            break;
        case 'a':
            opt.addr_string = optarg;
            break;
        case 'p':
            opt.port = atoi(optarg);
            break;
        case 'c':
            opt.amount = atoi(optarg);
            break;
        case 't':
            opt.jobs_count = strtoul(optarg, nullptr, 10);
            break;
        case 'e':
            opt.epoll_timeout = atoi(optarg);
            break;
        case 'i':
            opt.interval = atol(optarg);
            break;
        case 'o':
            opt.clock_timeout = atoi(optarg);
            break;
        case 'h':
        case '?':
            print_help(argv[0]);
            return -1;
        default:
            print("未知错误。\n");
            print_help(argv[0]);
            return argc;
        }
    }
    return 0;
}
