#include <arpa/inet.h>
#include <options.h>
#include <server.h>
#include <sf/color.hpp>
#include <sf/format.hpp>

using namespace std;
using namespace sf;

int main(int argc, char** argv)
{
    options opt = default_options;
    int go = get_opt(argc, argv, opt);
    if (go < 0)
    {
        return 0;
    }
    else if (go > 0)
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
        print("参数的调整请使用命令 {0} {1} 查看。\n", make_color_arg(argv[0], bright_blue), make_color_arg("-h", cyan));
    }
    print("按{0}刷新模块，{1}清除超时连接，{2}结束服务器。\n",
          make_color_arg("r <回车>", cyan),
          make_color_arg("c <回车>", cyan),
          make_color_arg("q <回车>", cyan));
    cout.flush();

    char c;
    while ((c = getchar()) != 'q')
    {
        switch (c)
        {
        case 'r':
            println(make_color_arg("刷新模块...", bright_magenta));
            ser.refresh_modules();
            cout.flush();
            break;
        case 'c':
            println(make_color_arg("正在清理...", bright_magenta));
            ser.clean(ser.get_time_stamp() + 1);
            cout.flush();
            break;
        }
    }

    println(make_color_arg("正在停止，请耐心等待...", bright_magenta));
    ser.stop();
    return 0;
}
