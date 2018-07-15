#include "server.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <csignal>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include "html_content.h"
#include "read_modules.h"

using namespace std;

server::server(unsigned int amount, size_t rec, size_t doj) : amount(amount)
{
    printf("初始化Socket...\n");
    sock = socket(AF_INET, SOCK_STREAM, 0);
    printf("初始化Epoll...\n");
    event_list = new epoll_event[amount];
    printf("初始化线程池...\n");
    pool = new thread_pool<tuple<int, server *>>(process_job, amount, rec, doj);
    refresh_module();
}

server::~server()
{
    if (pool)
    {
        delete pool;
        pool = nullptr;
    }
    if (event_list)
    {
        delete[] event_list;
        event_list = nullptr;
    }
}

void server::refresh_module()
{
    printf("刷新模块...\n");
    vector<string> lines = read_modules_file();
    {
        modules.clear();
        lock_guard<mutex> locker(modules_mutex);
        for (string &line : lines)
        {
            istringstream iss(line);
            string key, module_name;
            iss >> key >> module_name;
            modules.insert(map<string, module>::value_type(key, module(module_name.c_str())));
        }
    }
}

void server::start(const sockaddr *addr, socklen_t len, int n)
{
    bind(sock, addr, len);
    listen(sock, n);
    printf("监听数：%d\n", n);

    epoll_fd = epoll_create(amount);
    epoll_event event;
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = sock;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock, &event) < 0)
    {
        printf("Epoll启动失败。\n");
        return;
    }

    loop_thread = thread(accept_loop, this);
}

void server::stop()
{
    printf("关闭Socket。\n");
    close(sock);
    printf("关闭Epoll。\n");
    close(epoll_fd);
    loop_thread.join();
}

void server::accept_loop(server *ser)
{
    const int TIMEOUT = 2000;
    while (true)
    {
        int ret = epoll_wait(ser->epoll_fd, ser->event_list, ser->amount, TIMEOUT);
        if (ret < 0)
        {
            printf("Epoll已关闭。\n");
            break;
        }
        else if (ret == 0)
        {
            printf("超时。\n");
            continue;
        }
        for (int i = 0; i < ret; i++)
        {
            if ((ser->event_list[i].events & EPOLLERR) || (ser->event_list[i].events & EPOLLHUP) || !(ser->event_list[i].events & EPOLLIN))
            {
                printf("Epoll错误。\n");
                goto bre;
            }
            if (ser->event_list[i].data.fd == ser->sock)
            {
                sockaddr_in paddr;
                socklen_t len = sizeof(sockaddr_in);
                int newsock = accept(ser->sock, (sockaddr *)&paddr, &len);
                if (newsock < 0)
                    continue;
                epoll_event e;
                e.data.fd = newsock;
                e.events = EPOLLIN | EPOLLET;
                epoll_ctl(ser->epoll_fd, EPOLL_CTL_ADD, newsock, &e);
            }
            else
            {
                tuple<int, server *> *tpl = new tuple<int, server *>((int)(ser->event_list[i].data.fd), ser);
                printf("正在排队...\n");
                ser->pool->post(tpl);
            }
        }
    }
bre:
    for (int i = 0; i < ser->amount; i++)
    {
        close(ser->event_list[i].data.fd);
    }
    close(ser->epoll_fd);
    close(ser->sock);
}

void server::process_job(tuple<int, server *> *tpl)
{
    int fd = get<0>(*tpl);
    server *pser = get<1>(*tpl);
    printf("正在处理请求...\n");
    signal(SIGPIPE, [](int fd) -> void { close(fd); });
    char buffer[4096];
    memset(buffer, 0, sizeof(buffer));
    ssize_t size;
    size = recv(fd, buffer, sizeof(buffer), 0);
    //printf("请求为：\n%s\n长度：%ld\n", buffer, size);
    if (size > 0)
    {
        if (size < sizeof(buffer))
            buffer[size] = '\0';
        html_content content(buffer);
        {
            lock_guard<mutex> locker(pser->modules_mutex);
            printf("开始发送。\n");
            size = content.send(fd, pser->modules);
            printf("发送结束。\n");
        }
        if (size < 0)
        {
            printf("信息发送失败。\n");
        }
        else
        {
            printf("信息已发送。\n");
        }
    }
    delete tpl;
}
