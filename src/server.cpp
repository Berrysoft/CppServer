﻿#include "server.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <csignal>
#include <fcntl.h>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include "html_content.h"
#include "read_modules.h"

using namespace std;

server::server(size_t amount, size_t doj) : amount(amount)
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
    event_list = new epoll_event[amount];
    printf("初始化线程池...\n");
    pool = new thread_pool<int, server *>(doj, process_job);
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
            printf("加载模块：%s\n", key.c_str());
        }
    }
}

void server::start(const sockaddr *addr, socklen_t len, int n, int epoll_timeout, timespec interval, int clock_timeout)
{
    bind(sock, addr, len);
    listen(sock, n);
    printf("监听数：%d\n", n);
    printf("监听Socket：%d.\n", sock);

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

    loop_thread = thread(accept_loop, epoll_timeout, clock_timeout, this);
}

void server::clean(unsigned long long ostamp)
{
    printf("开始清理。\n");
    {
        lock_guard<mutex> locker(clients_mutex);
        for (vector<fd_with_time>::iterator it = clients.begin(); it != clients.end();)
        {
            if (it->time < ostamp)
            {
                printf("清理%d。\n", it->fd);
                clients.erase(it);
                close(it->fd);
            }
            else
            {
                it++;
            }
        }
    }
    printf("清理完成。\n");
}

void server::stop()
{
    printf("关闭Socket。\n");
    close(sock);
    printf("关闭时钟。\n");
    close(timer_fd);
    printf("关闭Epoll。\n");
    close(epoll_fd);
    loop_thread.join();
}

void server::accept_loop(int epoll_timeout, int clock_timeout, server *ser)
{
    while (true)
    {
        int ret = epoll_wait(ser->epoll_fd, ser->event_list, ser->amount, epoll_timeout);
        if (ret < 0 && ret != EINTR)
        {
            printf("Epoll已关闭。\n");
            break;
        }
        else if (ret == 0)
        {
            //printf("超时。\n");
            continue;
        }
        //printf("接收到%d个事件。\n", ret);
        for (int i = 0; i < ret; i++)
        {
            if ((ser->event_list[i].events & EPOLLERR) || (ser->event_list[i].events & EPOLLHUP) || (ser->event_list[i].events & EPOLLRDHUP) || !(ser->event_list[i].events & EPOLLIN))
            {
                if (ser->event_list[i].events & EPOLLRDHUP)
                {
                    printf("客户端已关闭Socket %d。\n", ser->event_list[i].data.fd);
                }
                else
                {
                    printf("Epoll错误，关闭Socket %d。\n", ser->event_list[i].data.fd);
                }
                epoll_ctl(ser->epoll_fd, EPOLL_CTL_DEL, ser->event_list[i].data.fd, &(ser->event_list[i]));
                close(ser->event_list[i].data.fd);
                {
                    lock_guard<mutex> locker(ser->clients_mutex);
                    for (vector<fd_with_time>::iterator it = ser->clients.begin(); it != ser->clients.end();)
                    {
                        if (it->fd == ser->event_list[i].data.fd)
                        {
                            ser->clients.erase(it);
                            close(it->fd);
                        }
                        else
                        {
                            it++;
                        }
                    }
                }
                continue;
            }
            if (ser->event_list[i].data.fd == ser->sock)
            {
                sockaddr_in paddr;
                socklen_t len = sizeof(sockaddr_in);
                int newsock;
                if ((newsock = accept(ser->sock, (sockaddr *)&paddr, &len)) > 0)
                {
                    printf("新建Socket：%d.\n", newsock);
                    int flags = fcntl(newsock, F_GETFL, 0);
                    fcntl(newsock, F_GETFL, flags | O_NONBLOCK);
                    epoll_event e;
                    e.data.fd = newsock;
                    e.events = EPOLLIN | EPOLLRDHUP | EPOLLET;
                    if (epoll_ctl(ser->epoll_fd, EPOLL_CTL_ADD, newsock, &e) < 0)
                    {
                        printf("接收%d错误。\n", newsock);
                    }
                    else
                    {
                        {
                            lock_guard<mutex> locker(ser->clients_mutex);
                            ser->clients.push_back({newsock, ser->time_stamp});
                        }
                    }
                }
            }
            else if (ser->event_list[i].data.fd == ser->timer_fd)
            {
                unsigned long long timer_buf;
                ssize_t len = read(ser->timer_fd, &timer_buf, sizeof(timer_buf));
                if (len < 0)
                {
                    continue;
                }
                ser->time_stamp += timer_buf;
                printf("时间戳：%llu\n", ser->time_stamp);
                if (ser->time_stamp > clock_timeout)
                {
                    ser->clean(ser->time_stamp - clock_timeout);
                }
            }
            else
            {
                int fd = (int)(ser->event_list[i].data.fd);
                {
                    lock_guard<mutex> locker(ser->clients_mutex);
                    for (vector<fd_with_time>::iterator it = ser->clients.begin(); it != ser->clients.end(); it++)
                    {
                        if (it->fd == fd)
                        {
                            it->time = ser->time_stamp;
                        }
                    }
                }
                ser->pool->post(fd, ser);
            }
        }
    }
    for (int i = 0; i < ser->amount; i++)
    {
        close(ser->event_list[i].data.fd);
    }
    close(ser->epoll_fd);
    close(ser->sock);
}

void server::process_job(int fd, server *pser)
{
    printf("正在处理请求%d...\n", fd);
    signal(SIGPIPE, SIG_IGN);
    char buffer[4096];
    memset(buffer, 0, sizeof(buffer));
    ssize_t size;
    size = recv(fd, buffer, sizeof(buffer), 0);
    if (size > 0)
    {
        if (size < sizeof(buffer))
            buffer[size] = '\0';
        html_content content(buffer);
        {
            lock_guard<mutex> locker(pser->modules_mutex);
            size = content.send(fd, pser->modules);
        }
        if (size < 0)
        {
            printf("信息发送失败%d。\n", fd);
        }
        else
        {
            printf("信息已发送%d。\n", fd);
        }
    }
}
