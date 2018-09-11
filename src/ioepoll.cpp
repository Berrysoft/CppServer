#include <ioepoll.h>
#include <unistd.h>

using namespace std;

ioepoll::ioepoll(ioepoll&& poll)
    : epoll_fd(poll.epoll_fd), amount(poll.amount), event_list(move(poll.event_list)), error_handler(move(poll.error_handler)), default_handler(move(poll.default_handler)), handlers(move(poll.handlers))
{
}

ioepoll& ioepoll::operator=(ioepoll&& poll)
{
    epoll_fd = poll.epoll_fd;
    amount = poll.amount;
    event_list = move(poll.event_list);
    error_handler = move(poll.error_handler);
    default_handler = move(poll.default_handler);
    handlers = move(poll.handlers);
    return *this;
}

int ioepoll::create(int amount)
{
    this->amount = amount;
    event_list = make_unique<epoll_event[]>(amount);
    return epoll_fd = epoll_create(amount);
}

void ioepoll::start(int timeout)
{
    while (true)
    {
        int ret = epoll_wait(epoll_fd, event_list.get(), amount, timeout);
        if (ret < 0)
        {
            break;
        }
        else if (ret == 0)
        {
            continue;
        }
        for (int i = 0; i < ret; i++)
        {
            int fd = event_list[i].data.fd;
            uint32_t events = event_list[i].events;
            if ((events & EPOLLERR) || (events & EPOLLHUP) || (events & EPOLLRDHUP) || !(events & EPOLLIN))
            {
                if (error_handler)
                    error_handler(fd, events);
            }
            else
            {
                auto it = handlers.find(fd);
                if (it != handlers.end())
                {
                    it->second(fd, events);
                }
                else
                {
                    if (default_handler)
                        default_handler(fd, events);
                }
            }
        }
    }
}

void ioepoll::close()
{
    for (int i = 0; i < amount; i++)
    {
        int fd = event_list[i].data.fd;
        if (fd != STDIN_FILENO && fd != STDOUT_FILENO && fd != STDERR_FILENO)
        {
            ::close(fd);
        }
    }
    ::close(epoll_fd);
}

int ioepoll::add(int fd, uint32_t events)
{
    return ctl(fd, events, EPOLL_CTL_ADD);
}

int ioepoll::modify(int fd, uint32_t events)
{
    return ctl(fd, events, EPOLL_CTL_MOD);
}

int ioepoll::remove(int fd, uint32_t events)
{
    return ctl(fd, events, EPOLL_CTL_DEL);
}

int ioepoll::ctl(int fd, uint32_t events, int op)
{
    epoll_event e;
    e.data.fd = fd;
    e.events = events;
    return epoll_ctl(epoll_fd, op, fd, &e);
}

void ioepoll::set_error_handler(handler_type handler)
{
    error_handler = handler;
}

void ioepoll::set_default_handler(handler_type handler)
{
    default_handler = handler;
}

void ioepoll::set_handler(int fd, handler_type handler)
{
    if (handler)
    {
        handlers.emplace(fd, handler);
    }
    else
    {
        auto it = handlers.find(fd);
        if (it != handlers.end())
        {
            handlers.erase(it);
        }
    }
}
