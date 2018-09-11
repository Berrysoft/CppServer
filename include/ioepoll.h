//封装Epoll
#pragma once
#include <functional>
#include <map>
#include <memory>
#include <sys/epoll.h>

class ioepoll
{
public:
    typedef std::function<void(int, uint32_t)> handler_type;

private:
    int epoll_fd;
    int amount;
    std::unique_ptr<epoll_event[]> event_list;
    handler_type error_handler;
    handler_type default_handler;
    std::map<int, handler_type> handlers;

public:
    ioepoll() = default;
    ioepoll(const ioepoll&) = delete;
    ioepoll(ioepoll&& poll);

    ioepoll& operator=(const ioepoll&) = delete;
    ioepoll& operator=(ioepoll&& poll);

    int create(int amount);
    void start(int timeout);
    void close();

    int add(int fd, uint32_t events);
    int modify(int fd, uint32_t events);
    int remove(int fd, uint32_t events);

private:
    int ctl(int fd, uint32_t events, int op);

public:
    void set_error_handler(handler_type handler);
    void set_default_handler(handler_type handler);
    void set_handler(int fd, handler_type handler);
};
