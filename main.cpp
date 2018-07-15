﻿#include <cstdio>
#include <cstring>
#include <arpa/inet.h>
#include "server.h"

using std::getchar;
using std::printf;

int main()
{
    const int amount = 7, m = 10, n = 4;
    const int port = 3342;

    server ser(amount, m, n);

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    printf("正在监听所有IP的%d端口。\n", port);
    printf("最大任务缓存数：%d\n最大任务接收数：%d\n最大任务处理数：%d\n", amount, m, n);
    
    ser.start((const sockaddr *)&address, sizeof(address), m);

    printf("请在自己的电脑上访问 http://127.0.0.1:%d/\n", port);
    printf("网站在Edge、IE、Chrome、Safari(iPhone)下测试通过。\n");
    printf("按r <回车>刷新模块，q <回车>结束服务器。\n");

    char c;
    while ((c = getchar()) != 'q')
    {
        switch (c)
        {
        case 'r':
            ser.refresh_module();
            break;
        }
    }

    ser.stop();
    return 0;
}