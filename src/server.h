//服务器头文件，管理一个循环线程和一个线程池。
#pragma once
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <thread>
#include <map>
#include <string>
#include <mutex>
#include <vector>
#include "thread_pool.h"
#include "module.h"

struct fd_with_time
{
    int fd;
    unsigned long long time;
};

class server
{
private:
    bool verbose;
    int sock;
    thread_pool<int, server*> *pool;
    std::thread loop_thread;
    std::map<std::string, module> modules;
    std::mutex modules_mutex;
    int epoll_fd;
    std::size_t amount;
    epoll_event *event_list;
    int timer_fd;
    unsigned long long time_stamp;
    std::vector<fd_with_time> clients;
    std::mutex clients_mutex;
public:
    server(std::size_t amount, std::size_t doj, bool verbose);
    ~server();

    void refresh_module();

    void start(const sockaddr *addr, socklen_t len, int n, int epoll_timeout = 2000, timespec interval = {60, 0}, int clock_timeout = 2);
    void clean(unsigned long long ostamp);
    void stop();

    unsigned long long get_time_stamp() { return time_stamp; }
private:
    static void accept_loop(int epoll_timeout, int clock_timeout, server *ser);
    static void process_job(int fd, server *pser);
};
