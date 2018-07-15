#pragma once
#include <netinet/in.h>
#include <sys/epoll.h>
#include <thread>
#include <map>
#include <string>
#include <tuple>
#include <mutex>
#include <vector>
#include "thread_pool.h"
#include "module.h"

class server
{
private:
    int sock;
    thread_pool<std::tuple<int, server*>> *pool;
    std::thread loop_thread;
    std::map<std::string, module> modules;
    std::mutex modules_mutex;
    int epoll_fd;
    unsigned int amount;
    epoll_event *event_list;
public:
    server(unsigned int amount, std::size_t rec, std::size_t doj);
    ~server();

    void refresh_module();

    void start(const sockaddr* addr, socklen_t len, int n);
    void stop();
private:
    static void accept_loop(server *ser);
    static void process_job(std::tuple<int, server *> *tpl);
};
