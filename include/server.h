//服务器头文件。
#pragma once
#include <exception>
#include <http/http.h>
#include <ioepoll.h>
#include <map>
#include <memory>
#include <mutex>
#include <netinet/in.h>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <thread>
#include <thread_pool.h>

struct server_exception : std::exception
{
private:
    std::string msg;

public:
    server_exception(std::string_view msg) : msg(msg) {}
    ~server_exception() override {}

    const char* what() const noexcept override { return msg.c_str(); }
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
    std::map<int, int> clients;
    mutable std::mutex clients_mutex;

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
    constexpr int get_time_stamp() const noexcept { return time_stamp; }

private:
    void epoll_error(int fd, std::uint32_t events);
    void epoll_sock(int fd, std::uint32_t events);
    void epoll_timer(int fd, std::uint32_t events);
    void epoll_default(int fd, std::uint32_t events);
    void process_job(int fd);
};
