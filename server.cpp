#include "server.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <csignal>
#include <fcntl.h>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include "html_content.h"
#include "read_modules.h"
#include "html_writer.h"

using namespace std;

server::server(size_t amount, size_t doj) : amount(amount)
{
    printf("初始化Socket...\n");
    sock = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    printf("初始化Epoll...\n");
    event_list = new epoll_event[amount];
    printf("初始化线程池...\n");
    pool = new thread_pool<tuple<int, server *>>(doj, process_job);
    refresh_module();
}

server::~server()
{
    if (pool)
    {
        delete pool;
        pool = nullptr;
    }
    if (event_list)
    {
        delete[] event_list;
        event_list = nullptr;
    }
}

void server::refresh_module()
{
    printf("刷新模块...\n");
    vector<string> lines = read_modules_file();
    {
        modules.clear();
        lock_guard<mutex> locker(modules_mutex);
        for (string &line : lines)
        {
            istringstream iss(line);
            string key, module_name;
            iss >> key >> module_name;
            modules.insert(map<string, module>::value_type(key, module(module_name.c_str())));
        }
    }
}

void server::start(const sockaddr *addr, socklen_t len, int n)
{
    bind(sock, addr, len);
    listen(sock, n);
    printf("监听数：%d\n", n);
    printf("Main socket: %d.\n", sock);

    epoll_fd = epoll_create(amount);
    epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = sock;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock, &event) < 0)
    {
        printf("Epoll启动失败。\n");
        return;
    }

    loop_thread = thread(accept_loop, this);
}

void server::stop()
{
    printf("关闭Socket。\n");
    close(sock);
    printf("关闭Epoll。\n");
    close(epoll_fd);
    loop_thread.join();
}

void server::accept_loop(server *ser)
{
    const int TIMEOUT = 2000;
    sigset_t sigs;
    sigaddset(&sigs, SIGPIPE);
    while (true)
    {
        int ret = epoll_pwait(ser->epoll_fd, ser->event_list, ser->amount, TIMEOUT, &sigs);
        if (ret < 0 && ret != EINTR)
        {
            printf("Epoll已关闭。\n");
            break;
        }
        else if (ret == 0)
        {
            printf("超时。\n");
            continue;
        }
        printf("接收到%d个事件。\n", ret);
        for (int i = 0; i < ret; i++)
        {
            if ((ser->event_list[i].events & EPOLLERR) || (ser->event_list[i].events & EPOLLHUP) || (ser->event_list[i].events & EPOLLRDHUP) || !(ser->event_list[i].events & EPOLLIN))
            {
                printf("Epoll错误，关闭Socket %d。\n", ser->event_list[i].data.fd);
                epoll_ctl(ser->epoll_fd, EPOLL_CTL_DEL, ser->event_list[i].data.fd, &(ser->event_list[i]));
                close(ser->event_list[i].data.fd);
                continue;
            }
            if (ser->event_list[i].data.fd == ser->sock)
            {
                sockaddr_in paddr;
                socklen_t len = sizeof(sockaddr_in);
                int newsock;
                if ((newsock = accept(ser->sock, (sockaddr *)&paddr, &len)) > 0)
                {
                    printf("新建Socket：%d.\n", newsock);
                    int flags = fcntl(newsock, F_GETFL, 0);
                    fcntl(newsock, F_GETFL, flags | O_NONBLOCK);
                    epoll_event e;
                    e.data.fd = newsock;
                    e.events = EPOLLIN | EPOLLRDHUP | EPOLLET;
                    if (epoll_ctl(ser->epoll_fd, EPOLL_CTL_ADD, newsock, &e) < 0)
                    {
                        printf("接收%d错误。\n", newsock);
                    }
                }
            }
            else
            {
                tuple<int, server *> *tpl = new tuple<int, server *>((int)(ser->event_list[i].data.fd), ser);
                printf("正在排队%d...\n", ser->event_list[i].data.fd);
                ser->pool->post(tpl);
            }
        }
    }
    for (int i = 0; i < ser->amount; i++)
    {
        close(ser->event_list[i].data.fd);
    }
    close(ser->epoll_fd);
    close(ser->sock);
}

void server::process_job(tuple<int, server *> *tpl)
{
    int fd = get<0>(*tpl);
    server *pser = get<1>(*tpl);
    printf("正在处理请求%d...\n", fd);
    signal(SIGPIPE, SIG_IGN);
    char buffer[4096];
    memset(buffer, 0, sizeof(buffer));
    ssize_t size;
    do
    {
        size = read(fd, buffer, sizeof(buffer));
        printf("%d请求长度为%ld。\n", fd, size);
    } while (size == sizeof(buffer));
    if (size > 0)
    {
        if (size < sizeof(buffer))
            buffer[size] = '\0';
        html_content content(buffer);
        {
            lock_guard<mutex> locker(pser->modules_mutex);
            printf("开始发送%d。\n", fd);
            try
            {
                size = content.send(fd, pser->modules);
            }
            catch (const broken_pipe &)
            {
                printf("%d异常断开。\n", fd);
                close(fd);
                pser->pool->remove(tpl);
                size = -1;
            }
            printf("发送结束%d。\n", fd);
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
    delete tpl;
}
