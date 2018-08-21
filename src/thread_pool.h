//线程池模板类。
#pragma once
#include <valarray>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <tuple>
#include <memory>
#include <functional>
#include "safe_queue.h"

template <typename... TArgs>
class thread_pool
{
private:
    std::function<void(TArgs...)> task;
    std::valarray<std::thread> do_threads;

    safe_queue<std::tuple<TArgs...>> jobs;
    std::condition_variable cond;
    std::mutex cond_mutex;

    std::atomic<bool> stop;
public:
    thread_pool(std::size_t dojob, std::function<void(TArgs...)> task);
    ~thread_pool();

    void post(TArgs... args);
private:
    void do_job();
};

template <typename... TArgs>
thread_pool<TArgs...>::thread_pool(std::size_t dojob, std::function<void(TArgs...)> task) : task(task)
{
    stop = false;
    if (dojob < 1)
        dojob = 1;
    do_threads = std::valarray<std::thread>(dojob);
    for (std::size_t i = 0; i < dojob; i++)
    {
        do_threads[i] = std::thread(std::bind(&thread_pool<TArgs...>::do_job, this));
    }
}

template <typename... TArgs>
thread_pool<TArgs...>::~thread_pool()
{
    stop = true;
    cond.notify_all();
    for (std::thread &t : do_threads)
    {
        t.join();
    }
}

template <typename... TArgs>
void thread_pool<TArgs...>::post(TArgs... args)
{
    jobs.emplace(args...);
    cond.notify_one();
}

template <typename... TArgs>
void thread_pool<TArgs...>::do_job()
{
    while (true)
    {
        if (jobs.empty())
        {
            std::unique_lock<std::mutex> locker(cond_mutex);
            cond.wait(locker);
        }
        if (stop)
            break;
        std::tuple<TArgs...> j;
        if (jobs.try_pop(j))
        {
            apply(task, j);
        }
    }
}
