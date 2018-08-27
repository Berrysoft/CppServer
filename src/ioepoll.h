#pragma once
#include <functional>
#include <map>
#include <memory>
#include <sys/epoll.h>

class ioepoll
{
private:
    int epoll_fd;
    int amount;
    std::unique_ptr<epoll_event[]> event_list;
    std::function<void(int, uint32_t)> error_handler;
    std::function<void(int, uint32_t)> default_handler;
    std::map<int, std::function<void(int, uint32_t)>> handlers;

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
    void set_error_handler(std::function<void(int, uint32_t)> handler);
    void set_default_handler(std::function<void(int, uint32_t)> handler);
    void set_handler(int fd, std::function<void(int, uint32_t)> handler);
    void remove_handler(int fd);
};
