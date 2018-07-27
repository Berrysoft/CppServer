#include <cstdio>
#include <cstring>
#include <arpa/inet.h>
#include <getopt.h>
#include "server.h"

using std::getchar;
using std::printf;

const char *const help_opt = "help";
const char *const verbose_opt = "verbose";
const char *const addr_opt = "address";
const char *const port_opt = "port";
const char *const count_opt = "count";
const char *const threads_opt = "threads";
const char *const etime_opt = "epoll-timeout";
const char *const cinterval_opt = "clock-interval";
const char *const ctime_opt = "clock-timeout";

int print_help(char* name)
{
    printf("Berrysoft.Linux.Cpp.Server\n");
    printf("网站在Edge、IE、Chrome、Safari(iPhone)下测试通过。\n");
    printf("请使用支持chunked的浏览器打开网站。\n");
    printf("用法：%s [-h] [-p %s] [-c %s] [-t %s]\n", name, port_opt, count_opt, threads_opt);
    printf("\t\t[%s] [%s] [%s]\n", etime_opt, cinterval_opt, ctime_opt);
    printf("选项：\n");
    printf("-h --%s\t\t获取帮助\n", help_opt);
    printf("-v --%s\t\t显示详细信息\n", verbose_opt);
    printf("-a --%s\t\t设置监听地址，默认为所有IP\n", addr_opt);
    printf("-p --%s\t\t设置监听端口，默认为3342\n", port_opt);
    printf("-c --%s\t\t设置监听数，默认为16384\n", count_opt);
    printf("-t --%s\t\t设置线程数，默认为4\n", threads_opt);
    printf("-e --%s\t设置Epoll的等待时间，默认为2000(ms)\n", etime_opt);
    printf("-i --%s\t设置时钟间隔，默认为60(s)\n", cinterval_opt);
    printf("-o --%s\t设置时钟等待循环数，默认为2（个）\n", ctime_opt);
    return 0;
}

int main(int argc, char **argv)
{
    bool verbose = false;
    char *addr_string = nullptr;
    int port = 3342;
    int amount = 16384;
    int n = 4;
    int epoll_timeout = 2000;
    timespec interval = {60, 0};
    int clock_timeout = 2;

    const char *const s_opts = "hva:p:c:t:e:i:o:";
    const option l_opts[] =
        {
            {help_opt, no_argument, nullptr, 'h'},
            {verbose_opt, no_argument, nullptr, 'v'},
            {addr_opt, required_argument, nullptr, 'a'},
            {port_opt, required_argument, nullptr, 'p'},
            {count_opt, required_argument, nullptr, 'c'},
            {threads_opt, required_argument, nullptr, 't'},
            {etime_opt, required_argument, nullptr, 'e'},
            {cinterval_opt, required_argument, nullptr, 'i'},
            {ctime_opt, required_argument, nullptr, 'o'},
            {nullptr, 0, nullptr, 0}
        };
    int opt;
    while ((opt = getopt_long(argc, argv, s_opts, l_opts, nullptr)) >= 0)
    {
        switch (opt)
        {
        case 'v':
            verbose = true;
            break;
        case 'a':
            addr_string = optarg;
            break;
        case 'p':
            port = atoi(optarg);
            break;
        case 'c':
            amount = atoi(optarg);
            break;
        case 't':
            n = atoi(optarg);
            break;
        case 'e':
            epoll_timeout = atoi(optarg);
            break;
        case 'i':
            interval.tv_sec = atoi(optarg);
            break;
        case 'o':
            clock_timeout = atoi(optarg);
            break;
        case 'h':
        case '?':
            return print_help(argv[0]);
        case -1:
            break;
        default:
            printf("未知错误。\n");
            print_help(argv[0]);
            return 1;
        }
    }

    in_addr_t listen_addr;
    if (!addr_string)
    {
        listen_addr = htonl(INADDR_ANY);
    }
    else
    {
        listen_addr = inet_addr(addr_string);
    }

    server ser(amount, n, verbose);

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = listen_addr;
    if (verbose)
        printf("正在监听所有IP的%d端口。\n", port);

    ser.start((const sockaddr *)&address, sizeof(address), amount, epoll_timeout, interval, clock_timeout);

    if (verbose)
    {
        printf("正在通过%d端口监听地址%s\n", port, inet_ntoa(*(in_addr *)&listen_addr));
        printf("参数的调整请使用命令 %s -h 查看。\n", argv[0]);
    }
    printf("按r <回车>刷新模块，c <回车>清除超时连接，q <回车>结束服务器。\n");

    char c;
    while ((c = getchar()) != 'q')
    {
        switch (c)
        {
        case 'r':
            ser.refresh_module();
            break;
        case 'c':
            ser.clean(ser.get_time_stamp() + 1);
            break;
        }
    }

    ser.stop();
    return 0;
}
