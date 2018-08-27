//服务器头文件，管理一个循环线程和一个线程池。
#pragma once
#include "http/http.h"
#include "ioepoll.h"
#include "thread_pool.h"
#include <map>
#include <memory>
#include <mutex>
#include <netinet/in.h>
#include <shared_mutex>
#include <string>
#include <thread>
#include <vector>

struct fd_with_time
{
    int fd;
    int time;
};

class server
{
private:
    bool verbose;
    int sock;
    thread_pool<int> pool;
    std::thread loop_thread;
    http http_parser;
    std::shared_mutex http_mutex;
    int amount;
    ioepoll epoll;
    int timer_fd;
    int clock_timeout;
    int time_stamp;
    std::vector<fd_with_time> clients;
    std::mutex clients_mutex;

public:
    server(int amount, std::size_t doj, bool verbose);
    server(const server&) = delete;
    server(server&&) = delete;
    ~server();

    server& operator=(const server&) = delete;
    server& operator=(server&&) = delete;

    void bind(const sockaddr_in& addr, int n);
    void bind(const sockaddr_in6& addr, int n);

private:
    void bind(const sockaddr* addr, socklen_t len, int n);

public:
    void start(int epoll_timeout, long interval, int clock_timeout);
    void clean(int ostamp);
    void stop();

    void refresh_modules();
    int get_time_stamp() { return time_stamp; }

private:
    void epoll_error(int fd, uint32_t events);
    void epoll_sock(int fd, uint32_t events);
    void epoll_timer(int fd, uint32_t events);
    void epoll_default(int fd, uint32_t events);
    void process_job(int fd);
};
