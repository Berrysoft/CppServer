#include <cstdio>
#include <string>
#include <arpa/inet.h>
#include <getopt.h>
#include "server.h"

using namespace std;

const char help_opt[] = "help";
const char verbose_opt[] = "verbose";
const char addr_opt[] = "address";
const char port_opt[] = "port";
const char count_opt[] = "count";
const char threads_opt[] = "threads";
const char etime_opt[] = "epoll-timeout";
const char cinterval_opt[] = "clock-interval";
const char ctime_opt[] = "clock-timeout";

int print_help(const char *name)
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
    size_t n = 4;
    int epoll_timeout = 2000;
    timespec interval = {60, 0};
    int clock_timeout = 2;

    const char s_opts[] = "hva:p:c:t:e:i:o:";
    const option l_opts[] = {
        {help_opt, no_argument, nullptr, 'h'},
        {verbose_opt, no_argument, nullptr, 'v'},
        {addr_opt, required_argument, nullptr, 'a'},
        {port_opt, required_argument, nullptr, 'p'},
        {count_opt, required_argument, nullptr, 'c'},
        {threads_opt, required_argument, nullptr, 't'},
        {etime_opt, required_argument, nullptr, 'e'},
        {cinterval_opt, required_argument, nullptr, 'i'},
        {ctime_opt, required_argument, nullptr, 'o'},
        {nullptr, 0, nullptr, 0}};
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
            port = stoi(optarg);
            break;
        case 'c':
            amount = stoi(optarg);
            break;
        case 't':
            n = stoul(optarg);
            break;
        case 'e':
            epoll_timeout = stoi(optarg);
            break;
        case 'i':
            interval.tv_sec = stol(optarg);
            break;
        case 'o':
            clock_timeout = stoi(optarg);
            break;
        case 'h':
        case '?':
            return print_help(argv[0]);
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

    ser.bind(address, amount);
    ser.start(epoll_timeout, interval, clock_timeout);

    if (verbose)
    {
        if (addr_string)
        {
            printf("正在通过%d端口监听地址%s\n", port, addr_string);
        }
        else
        {
            printf("正在通过%d端口监听所有IP地址。\n", port);
        }
        printf("参数的调整请使用命令 %s -h 查看。\n", argv[0]);
    }
    printf("按r <回车>刷新模块，c <回车>清除超时连接，q <回车>结束服务器。\n");

    char c;
    while ((c = getchar()) != 'q')
    {
        switch (c)
        {
        case 'r':
            printf("刷新模块...\n");
            ser.refresh_modules();
            break;
        case 'c':
            printf("正在清理...\n");
            ser.clean(ser.get_time_stamp() + 1);
            break;
        }
    }

    ser.stop();
    return 0;
}
