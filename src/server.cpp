#include "server.h"
#include "http/http_request.h"
#include <arpa/inet.h>
#include <csignal>
#include <cstdio>
#include <fcntl.h>
#include <fstream>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

#define printf(exp, ...)                                                       \
    if (verbose)                                                               \
    {                                                                          \
        std::printf(exp, ##__VA_ARGS__);                                       \
    }

#define NEGATIVE_RETURN(exp, msg)                                              \
    if ((exp) < 0)                                                             \
    {                                                                          \
        printf(msg);                                                           \
        return;                                                                \
    }

using namespace std;

server::server(size_t amount, size_t doj, bool verbose)
    : verbose(verbose), amount(amount)
{
    printf("初始化Socket...\n");
    sock = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    NEGATIVE_RETURN(sock, "Socket初始化失败。\n");
    printf("初始化时钟...\n");
    time_stamp = 0;
    timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
    NEGATIVE_RETURN(timer_fd, "时钟初始化失败。\n");
    printf("初始化Epoll...\n");
    event_list = unique_ptr<epoll_event[]>(new epoll_event[amount]);
    printf("初始化线程池...\n");
    pool.start(doj, mem_fn(&server::process_job));
    printf("刷新模块...\n");
    refresh_modules();
}

server::~server()
{
    printf("关闭Socket。\n");
    close(sock);
    printf("关闭时钟。\n");
    close(timer_fd);
}

void server::bind(const sockaddr_in& addr, int n)
{
    bind((const sockaddr*)&addr, sizeof(addr), n);
}

void server::bind(const sockaddr_in6& addr, int n)
{
    bind((const sockaddr*)&addr, sizeof(addr), n);
}

void server::bind(const sockaddr* addr, socklen_t len, int n)
{
    NEGATIVE_RETURN(::bind(sock, addr, len), "Socket命名失败。\n");
    NEGATIVE_RETURN(listen(sock, n), "监听失败。\n");
    printf("监听数：%d\n", n);
    printf("监听Socket：%d.\n", sock);
}

void server::start(int epoll_timeout, timespec& interval, int clock_timeout)
{
    itimerspec itimer;
    NEGATIVE_RETURN(clock_gettime(CLOCK_MONOTONIC, &itimer.it_value),
                    "时钟获取失败。\n");
    itimer.it_value = interval;
    itimer.it_interval = interval;
    NEGATIVE_RETURN(timerfd_settime(timer_fd, 0, &itimer, nullptr),
                    "时钟设置失败。\n");

    epoll_fd = epoll_create(amount);
    NEGATIVE_RETURN(epoll_fd, "Epoll启动失败。\n");

    printf("Epoll等待时间：%d(ms)\n", epoll_timeout);
    printf("时钟间隔：%ld(s)\n", interval.tv_sec);
    printf("时钟等待循环数：%d（个）\n", clock_timeout);

    epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = sock;
    NEGATIVE_RETURN(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock, &event),
                    "Socket启动失败。\n");

    event.events = EPOLLIN | EPOLLET;
    event.data.fd = timer_fd;
    NEGATIVE_RETURN(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, timer_fd, &event),
                    "时钟启动失败。\n");

    loop_thread = thread(mem_fn(&server::accept_loop), this, epoll_timeout,
                         clock_timeout);
}

void server::clean(int ostamp)
{
    lock_guard<mutex> locker(clients_mutex);
    for (vector<fd_with_time>::iterator it = clients.begin();
         it != clients.end();)
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
    printf("关闭Epoll。\n");
    close(epoll_fd);
    puts("停止循环，请耐心等待...");
    loop_thread.join();
    printf("停止线程池。\n");
    pool.stop();
}

void server::refresh_modules()
{
    unique_lock<shared_mutex> locker(http_mutex);
    http_parser.refresh_modules();
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
            if ((event_list[i].events & EPOLLERR) ||
                (event_list[i].events & EPOLLHUP) ||
                (event_list[i].events & EPOLLRDHUP) ||
                !(event_list[i].events & EPOLLIN))
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
                    for (vector<fd_with_time>::iterator it = clients.begin();
                         it != clients.end();)
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
                if ((newsock = accept(sock, (sockaddr*)&paddr, &len)) > 0)
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
                        lock_guard<mutex> locker(clients_mutex);
                        clients.push_back({newsock, time_stamp});
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
                    for (vector<fd_with_time>::iterator it = clients.begin();
                         it != clients.end(); it++)
                    {
                        if (it->fd == fd)
                        {
                            it->time = time_stamp;
                        }
                    }
                }
                pool.post(this, fd);
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
    unique_ptr<http_response> response;
    {
        shared_lock<shared_mutex> locker(http_mutex);
        response = http_parser.get_response(request);
    }
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
