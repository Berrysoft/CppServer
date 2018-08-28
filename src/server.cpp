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
#include <sys/timerfd.h>
#include <unistd.h>

#define printf(exp, ...)                 \
    if (verbose)                         \
    {                                    \
        std::printf(exp, ##__VA_ARGS__); \
    }

#define NEGATIVE_RETURN(exp, msg) \
    if ((exp) < 0)                \
    {                             \
        printf(msg);              \
        return;                   \
    }

using namespace std;
using std::placeholders::_1;
using std::placeholders::_2;

server::server(int amount, size_t doj, bool verbose)
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
    NEGATIVE_RETURN(epoll.create(amount), "Epoll启动失败。\n");
    printf("初始化线程池...\n");
    pool.start(doj, std::bind(&server::process_job, this, _1));
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

void server::start(int epoll_timeout, long interval, int clock_timeout)
{
    itimerspec itimer;
    NEGATIVE_RETURN(clock_gettime(CLOCK_MONOTONIC, &itimer.it_value), "时钟获取失败。\n");
    itimer.it_value = { interval, 0 };
    itimer.it_interval = { interval, 0 };
    NEGATIVE_RETURN(timerfd_settime(timer_fd, 0, &itimer, nullptr), "时钟设置失败。\n");
    this->clock_timeout = clock_timeout;

    printf("Epoll等待时间：%d(ms)\n", epoll_timeout);
    printf("时钟间隔：%ld(s)\n", interval);
    printf("时钟等待循环数：%d（个）\n", clock_timeout);

    NEGATIVE_RETURN(epoll.add(sock, EPOLLIN), "Socket启动失败。\n");

    NEGATIVE_RETURN(epoll.add(timer_fd, EPOLLIN | EPOLLET), "时钟启动失败。\n");

    epoll.set_error_handler(std::bind(&server::epoll_error, this, _1, _2));
    epoll.set_default_handler(std::bind(&server::epoll_default, this, _1, _2));
    epoll.set_handler(sock, std::bind(&server::epoll_sock, this, _1, _2));
    epoll.set_handler(timer_fd, std::bind(&server::epoll_timer, this, _1, _2));

    loop_thread = thread(&ioepoll::start, &epoll, epoll_timeout);
}

void server::clean(int ostamp)
{
    lock_guard<mutex> locker(clients_mutex);
    for (auto it = clients.begin(); it != clients.end();)
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
    epoll.close();
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

void server::epoll_error(int fd, uint32_t events)
{
    if (events & EPOLLRDHUP)
    {
        printf("客户端已关闭Socket %d。\n", fd);
    }
    else
    {
        printf("Epoll错误，关闭Socket %d。\n", fd);
    }
    epoll.remove(fd, events);
    shutdown(fd, SHUT_RDWR);
    close(fd);
    {
        lock_guard<mutex> locker(clients_mutex);
        for (auto it = clients.begin(); it != clients.end();)
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
}

void server::epoll_sock(int fd, uint32_t events)
{
    sockaddr_in paddr;
    socklen_t len = sizeof(sockaddr_in);
    int newsock;
    if ((newsock = accept(sock, (sockaddr*)&paddr, &len)) > 0)
    {
        printf("新建Socket：%d.\n", newsock);
        int flags = fcntl(newsock, F_GETFL, 0);
        fcntl(newsock, F_GETFL, flags | O_NONBLOCK);
        if (epoll.add(newsock, EPOLLIN | EPOLLRDHUP | EPOLLET))
        {
            printf("接收%d错误。\n", newsock);
        }
        else
        {
            lock_guard<mutex> locker(clients_mutex);
            clients.push_back({ newsock, time_stamp });
        }
    }
}

void server::epoll_timer(int fd, uint32_t events)
{
    unsigned long long timer_buf;
    ssize_t len = read(timer_fd, &timer_buf, sizeof(timer_buf));
    if (len >= 0)
    {
        time_stamp += timer_buf;
        printf("时间戳：%d\n", time_stamp);
        if (time_stamp > clock_timeout)
        {
            clean(time_stamp - clock_timeout);
            time_stamp = 0;
        }
    }
}

void server::epoll_default(int fd, uint32_t events)
{
    {
        lock_guard<mutex> locker(clients_mutex);
        for (auto it = clients.begin(); it != clients.end(); it++)
        {
            if (it->fd == fd)
            {
                it->time = time_stamp;
            }
        }
    }
    pool.post(fd);
}

void server::process_job(int fd)
{
    printf("正在处理请求%d...\n", fd);
    signal(SIGPIPE, SIG_IGN);
    optional<http_request> request = http_request::parse(fd);
    ssize_t size = -1;
    if (request)
    {
        {
            shared_lock<shared_mutex> locker(http_mutex);
            size = http_parser.send(fd, *request);
        }
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
