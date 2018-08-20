﻿#include "server.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <csignal>
#include <fcntl.h>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include "http_request.h"

#define printf(exp, ...) \
    if (verbose)         \
    std::printf(exp, ##__VA_ARGS__)

using namespace std;
using std::placeholders::_1;
using std::placeholders::_2;

server::server(size_t amount, size_t doj, bool verbose) : verbose(verbose), amount(amount)
{
    printf("初始化Socket...\n");
    sock = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (sock < 0)
    {
        printf("Socket初始化失败。\n");
        return;
    }
    printf("初始化时钟...\n");
    time_stamp = 0;
    timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
    if (timer_fd < 0)
    {
        printf("时钟初始化失败。\n");
        return;
    }
    printf("初始化Epoll...\n");
    event_list = unique_ptr<epoll_event[]>(new epoll_event[amount]);
    printf("初始化线程池...\n");
    pool = make_unique<thread_pool<int>>(doj, bind(&server::process_job, this, _1));
    printf("刷新模块...\n");
    refresh_module();
}

void server::refresh_module()
{
    lock_guard<mutex> locker(http_mutex);
    http_parser.refresh_modules();
}

void server::start(const sockaddr *addr, socklen_t len, int n, int epoll_timeout, timespec interval, int clock_timeout)
{
    bind(sock, addr, len);
    listen(sock, n);
    printf("监听数：%d\n", n);
    printf("监听Socket：%d.\n", sock);
    printf("Epoll等待时间：%d(ms)\n", epoll_timeout);
    printf("时钟间隔：%ld(s)\n", interval.tv_sec);
    printf("时钟等待循环数：%d（个）\n", clock_timeout);

    itimerspec itimer;
    if (clock_gettime(CLOCK_MONOTONIC, &itimer.it_value) < 0)
    {
        printf("时钟获取失败。\n");
        return;
    }
    itimer.it_value = interval;
    itimer.it_interval = interval;
    if (timerfd_settime(timer_fd, 0, &itimer, nullptr) < 0)
    {
        printf("时钟设置失败。\n");
        return;
    }

    epoll_fd = epoll_create(amount);
    epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = sock;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock, &event) < 0)
    {
        printf("Socket启动失败。\n");
        return;
    }

    event.events = EPOLLIN | EPOLLET;
    event.data.fd = timer_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, timer_fd, &event) < 0)
    {
        printf("时钟启动失败。\n");
        return;
    }

    loop_thread = thread(bind(&server::accept_loop, this, _1, _2), epoll_timeout, clock_timeout);
}

void server::clean(int ostamp)
{
    lock_guard<mutex> locker(clients_mutex);
    for (vector<fd_with_time>::iterator it = clients.begin(); it != clients.end();)
    {
        if (it->time < ostamp)
        {
            printf("清理%d。\n", it->fd);
            clients.erase(it);
            shutdown(it->fd, SHUT_RDWR);
            close(it->fd);
        }
        else
        {
            it++;
        }
    }
}

void server::stop()
{
    printf("关闭Socket。\n");
    close(sock);
    printf("关闭时钟。\n");
    close(timer_fd);
    printf("关闭Epoll。\n");
    close(epoll_fd);
    puts("停止循环，请耐心等待...");
    loop_thread.join();
}

void server::accept_loop(int epoll_timeout, int clock_timeout)
{
    while (true)
    {
        int ret = epoll_wait(epoll_fd, event_list.get(), amount, epoll_timeout);
        if (ret < 0 && ret != EINTR)
        {
            printf("Epoll已关闭。\n");
            break;
        }
        else if (ret == 0)
        {
            printf("超时。\n");
            continue;
        }
        printf("接收到%d个事件。\n", ret);
        for (int i = 0; i < ret; i++)
        {
            int fd = event_list[i].data.fd;
            if ((event_list[i].events & EPOLLERR) || (event_list[i].events & EPOLLHUP) || (event_list[i].events & EPOLLRDHUP) || !(event_list[i].events & EPOLLIN))
            {
                if (event_list[i].events & EPOLLRDHUP)
                {
                    printf("客户端已关闭Socket %d。\n", fd);
                }
                else
                {
                    printf("Epoll错误，关闭Socket %d。\n", fd);
                }
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, &(event_list[i]));
                shutdown(fd, SHUT_RDWR);
                close(fd);
                {
                    lock_guard<mutex> locker(clients_mutex);
                    for (vector<fd_with_time>::iterator it = clients.begin(); it != clients.end();)
                    {
                        if (it->fd == fd)
                        {
                            clients.erase(it);
                        }
                        else
                        {
                            it++;
                        }
                    }
                }
                continue;
            }
            if (fd == sock)
            {
                sockaddr_in paddr;
                socklen_t len = sizeof(sockaddr_in);
                int newsock;
                if ((newsock = accept(sock, (sockaddr *)&paddr, &len)) > 0)
                {
                    printf("新建Socket：%d.\n", newsock);
                    int flags = fcntl(newsock, F_GETFL, 0);
                    fcntl(newsock, F_GETFL, flags | O_NONBLOCK);
                    epoll_event e;
                    e.data.fd = newsock;
                    e.events = EPOLLIN | EPOLLRDHUP | EPOLLET;
                    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, newsock, &e) < 0)
                    {
                        printf("接收%d错误。\n", newsock);
                    }
                    else
                    {
                        {
                            lock_guard<mutex> locker(clients_mutex);
                            clients.push_back({newsock, time_stamp});
                        }
                    }
                }
            }
            else if (fd == timer_fd)
            {
                unsigned long long timer_buf;
                ssize_t len = read(timer_fd, &timer_buf, sizeof(timer_buf));
                if (len < 0)
                {
                    continue;
                }
                time_stamp += timer_buf;
                printf("时间戳：%d\n", time_stamp);
                if (time_stamp > clock_timeout)
                {
                    clean(time_stamp - clock_timeout);
                    time_stamp = 0;
                }
            }
            else
            {
                {
                    lock_guard<mutex> locker(clients_mutex);
                    for (vector<fd_with_time>::iterator it = clients.begin(); it != clients.end(); it++)
                    {
                        if (it->fd == fd)
                        {
                            it->time = time_stamp;
                        }
                    }
                }
                pool->post(fd);
            }
        }
    }
    for (size_t i = 0; i < amount; i++)
    {
        close(event_list[i].data.fd);
    }
    close(epoll_fd);
    close(sock);
}

void server::process_job(int fd)
{
    printf("正在处理请求%d...\n", fd);
    signal(SIGPIPE, SIG_IGN);
    http_request request = http_request::parse(fd);
    unique_ptr<http_response> response = http_parser.get_response(request);
    ssize_t size = response->send(fd);
    if (size < 0)
    {
        printf("信息发送失败%d。\n", fd);
    }
    else
    {
        printf("信息已发送%d。\n", fd);
    }
}
