#include "options.h"
#include "server.h"
#include <arpa/inet.h>
#include <fmt/core.h>

using namespace std;
using fmt::print;

int main(int argc, char** argv)
{
    options opt = default_options;
    if (get_opt(argc, argv, opt))
    {
        return 1;
    }

    in_addr_t listen_addr;
    if (!opt.addr_string)
    {
        listen_addr = htonl(INADDR_ANY);
    }
    else
    {
        listen_addr = inet_addr(opt.addr_string);
    }

    server ser(opt.amount, opt.jobs_count, opt.verbose);

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(opt.port);
    address.sin_addr.s_addr = listen_addr;

    ser.bind(address, opt.amount);
    ser.start(opt.epoll_timeout, opt.interval, opt.clock_timeout);

    if (opt.verbose)
    {
        if (opt.addr_string)
        {
            print("正在通过{0}端口监听地址{1}\n", opt.port, opt.addr_string);
        }
        else
        {
            print("正在通过{0}端口监听所有IP地址。\n", opt.port);
        }
        print("参数的调整请使用命令 {0} -h 查看。\n", argv[0]);
    }
    print("按r <回车>刷新模块，c <回车>清除超时连接，q <回车>结束服务器。\n");

    char c;
    while ((c = getchar()) != 'q')
    {
        switch (c)
        {
        case 'r':
            print("刷新模块...\n");
            ser.refresh_modules();
            break;
        case 'c':
            print("正在清理...\n");
            ser.clean(ser.get_time_stamp() + 1);
            break;
        }
    }

    ser.stop();
    return 0;
}
