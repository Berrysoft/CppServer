﻿#include "server.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <csignal>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include "html_content.h"
#include "read_modules.h"

using namespace std;

server::server(unsigned int amount, size_t rec, size_t doj)
{
    printf("初始化Socket...\n");
    sock = socket(AF_INET, SOCK_STREAM, 0);
    printf("初始化线程池...\n");
    pool = new thread_pool<tuple<int, server *>>(process_job, amount, rec, doj);
    refresh_module();
}

server::~server()
{
    if (pool)
    {
        delete pool;
        pool = nullptr;
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

void server::close_children()
{
    printf("关闭子Socket...\n");
    while (!child_sockets.empty())
    {
        vector<int>::iterator it = child_sockets.begin();
        close(*it);
        child_sockets.erase(it);
    }
}

void server::start(const sockaddr *addr, socklen_t len, int n)
{
    bind(sock, addr, len);
    listen(sock, n);
    printf("监听数：%d\n", n);
    loop_thread = thread(accept_loop, &sock, pool, this);
}

void server::stop()
{
    printf("关闭Socket。\n");
    close(sock);
    {
        lock_guard<mutex> locker(children_mutex);
        for (int &s : child_sockets)
        {
            close(s);
        }
    }
    printf("关闭线程池。\n");
    loop_thread.join();
}

void server::accept_loop(int *sock, thread_pool<tuple<int, server *>> *pool, server *ser)
{
    while (true)
    {
        sockaddr *paddr;
        socklen_t len;
        int newsock = accept(*sock, paddr, &len);
        if (newsock < 0)
            break;
        printf("接收到请求。\n");
        bool startnew = false;
        {
            lock_guard<mutex> locker(ser->children_mutex);
            vector<int>::iterator it = find(ser->child_sockets.begin(), ser->child_sockets.end(), newsock);
            if (it == ser->child_sockets.end())
            {
                ser->child_sockets.push_back(newsock);
                startnew = true;
            }
        }
        if (startnew)
        {
            tuple<int, server *> *tpl = new tuple<int, server *>(newsock, ser);
            printf("正在排队...\n");
            pool->post(tpl);
        }
    }
}

void server::process_job(tuple<int, server *> *tpl)
{
    int fd = get<0>(*tpl);
    server *pser = get<1>(*tpl);
    printf("正在处理请求...\n");
    signal(SIGPIPE, [](int fd) -> void { close(fd); });
    while (true)
    {
        char buffer[4096];
        memset(buffer, 0, sizeof(buffer));
        ssize_t size;
        size = recv(fd, buffer, sizeof(buffer), 0);
        //printf("请求为：\n%s\n长度：%ld\n", buffer, size);
        if (size < 0)
            break;
        else if (size > 0)
        {
            if (size < sizeof(buffer))
                buffer[size] = '\0';
            html_content content(buffer);
            {
                lock_guard<mutex> locker(pser->modules_mutex);
                printf("开始发送。\n");
                size = content.send(fd, pser->modules);
                printf("发送结束。\n");
            }
            if (size < 0)
            {
                printf("信息发送失败。\n");
            }
            else
            {
                printf("信息已发送。\n");
            }
        }
    }
    close(fd);
    {
        lock_guard<mutex> locker(pser->children_mutex);
        vector<int>::iterator it = find(pser->child_sockets.begin(), pser->child_sockets.end(), fd);
        if (it != pser->child_sockets.end())
        {
            pser->child_sockets.erase(it);
        }
    }
    delete tpl;
}
