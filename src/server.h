//服务器头文件，管理一个循环线程和一个线程池。
#pragma once
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <thread>
#include <map>
#include <string>
#include <mutex>
#include <shared_mutex>
#include <vector>
#include <memory>
#include "thread_pool.h"
#include "http/http.h"

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
    std::unique_ptr<thread_pool<server *, int>> pool;
    std::thread loop_thread;
    http http_parser;
    std::shared_mutex http_mutex;
    int epoll_fd;
    std::size_t amount;
    std::unique_ptr<epoll_event[]> event_list;
    int timer_fd;
    int time_stamp;
    std::vector<fd_with_time> clients;
    std::mutex clients_mutex;
public:
    server(std::size_t amount, std::size_t doj, bool verbose);
    ~server();

    void bind(const sockaddr_in &addr, int n);
    void bind(const sockaddr_in6 &addr, int n);
private:
    void bind(const sockaddr *addr, socklen_t len, int n);
public:
    void start(int epoll_timeout, timespec &interval, int clock_timeout);
    void clean(int ostamp);
    void stop();

    void refresh_modules();
    int get_time_stamp() { return time_stamp; }
private:
    void accept_loop(int epoll_timeout, int clock_timeout);
    void process_job(int fd);
};
