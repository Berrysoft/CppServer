﻿#include <arpa/inet.h>
#include <csignal>
#include <fcntl.h>
#include <http/http_request.h>
#include <server.h>
#include <sf/color.hpp>
#include <sf/format.hpp>
#include <sys/socket.h>
#include <sys/timerfd.h>
#include <unistd.h>

#define println(...)              \
    if (verbose)                  \
    {                             \
        sf::println(__VA_ARGS__); \
    }

#define NEGATIVE_RETURN(exp, msg)      \
    if ((exp) < 0)                     \
    {                                  \
        throw server_exception{ msg }; \
    }

using namespace std;
using namespace sf;

server::server(int amount, size_t doj, bool verbose)
    : verbose(verbose), amount(amount)
{
    println(make_color_arg("初始化Socket...", yellow));
    sock = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    NEGATIVE_RETURN(sock, "Socket初始化失败。");
    int enable = 1;
    NEGATIVE_RETURN(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)), "Socket属性设置失败");
    println(make_color_arg("初始化时钟...", yellow));
    time_stamp = 0;
    timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
    NEGATIVE_RETURN(timer_fd, "时钟初始化失败。");
    println(make_color_arg("初始化Epoll...", yellow));
    NEGATIVE_RETURN(epoll.create(amount), "Epoll创建失败。");
    println(make_color_arg("初始化线程池...", yellow));
    pool.start(doj, mem_fn_bind(&server::process_job, this));
    println(make_color_arg("刷新模块...", yellow));
    refresh_modules();
}

server::~server()
{
    println(make_color_arg("关闭Socket。", yellow));
    close(sock);
    println(make_color_arg("关闭时钟。", yellow));
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
    NEGATIVE_RETURN(::bind(sock, addr, len), "Socket绑定失败。");
    NEGATIVE_RETURN(listen(sock, n), "监听失败。");
    println("监听数：{0}", make_color_arg(n, bright_blue));
    println("监听Socket：{0}.", sock);
}

void server::start(int epoll_timeout, long interval, int clock_timeout)
{
    itimerspec itimer;
    NEGATIVE_RETURN(clock_gettime(CLOCK_MONOTONIC, &itimer.it_value), "时钟获取失败。");
    itimer.it_value = { interval, 0 };
    itimer.it_interval = { interval, 0 };
    NEGATIVE_RETURN(timerfd_settime(timer_fd, 0, &itimer, nullptr), "时钟设置失败。");
    this->clock_timeout = clock_timeout;

    println("Epoll等待时间：{0}(ms)", make_color_arg(epoll_timeout, bright_blue));
    println("时钟间隔：{0}(s)", make_color_arg(interval, bright_blue));
    println("时钟等待循环数：{0}（个）", make_color_arg(clock_timeout, bright_blue));

    NEGATIVE_RETURN(epoll.add(sock, EPOLLIN), "Socket启动失败。");

    NEGATIVE_RETURN(epoll.add(timer_fd, EPOLLIN | EPOLLET), "时钟启动失败。");

    epoll.set_error_handler(mem_fn_bind(&server::epoll_error, this));
    epoll.set_default_handler(mem_fn_bind(&server::epoll_default, this));
    epoll.set_handler(sock, mem_fn_bind(&server::epoll_sock, this));
    epoll.set_handler(timer_fd, mem_fn_bind(&server::epoll_timer, this));

    loop_thread = thread(&ioepoll::start, &epoll, epoll_timeout);
}

void server::clean(int ostamp)
{
    lock_guard<mutex> locker(clients_mutex);
    for (auto it = clients.begin(); it != clients.end();)
    {
        if (it->second < ostamp)
        {
            println("清理{0}。", it->first);
            shutdown(it->first, SHUT_RDWR);
            close(it->first);
            it = clients.erase(it);
        }
        else
        {
            it++;
        }
    }
}

void server::stop()
{
    println(make_color_arg("关闭Epoll。", yellow));
    epoll.close();
    println(make_color_arg("停止循环。", yellow));
    loop_thread.join();
    println(make_color_arg("停止线程池。", yellow));
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
        println("客户端已关闭Socket {0}。", fd);
    }
    else
    {
        println("Epoll错误，关闭Socket {0}。", fd);
    }
    epoll.remove(fd, events);
    shutdown(fd, SHUT_RDWR);
    close(fd);
    {
        lock_guard<mutex> locker(clients_mutex);
        clients.erase(fd);
    }
}

void server::epoll_sock(int fd, uint32_t events)
{
    sockaddr_in paddr;
    socklen_t len = sizeof(sockaddr_in);
    int newsock;
    if ((newsock = accept(sock, (sockaddr*)&paddr, &len)) > 0)
    {
        println("新建Socket：{0}", newsock);
        int flags = fcntl(newsock, F_GETFL, 0);
        fcntl(newsock, F_GETFL, flags | O_NONBLOCK);
        if (epoll.add(newsock, EPOLLIN | EPOLLRDHUP | EPOLLET))
        {
            println("接收{0}错误。", newsock);
        }
        else
        {
            lock_guard<mutex> locker(clients_mutex);
            clients[newsock] = time_stamp;
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
        println("时间戳：{0}", make_color_arg(time_stamp, bright_blue));
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
        clients[fd] = time_stamp;
    }
    pool.post(fd);
}

void server::process_job(int fd)
{
    println("正在处理请求{0}...", fd);
    signal(SIGPIPE, SIG_IGN);
    optional<http_request> request = http_request::parse(fd);
    ssize_t size = -1;
    if (request)
    {
        shared_lock<shared_mutex> locker(http_mutex);
        size = http_parser.send(fd, *request);
    }
    if (size < 0)
    {
        println("信息发送失败{0}。", fd);
    }
    else
    {
        println("信息已发送{0}。", fd);
    }
}
