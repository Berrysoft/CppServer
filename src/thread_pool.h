//线程池模板类。
#pragma once
#include "safe_queue.h"
#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <tuple>
#include <valarray>

template <typename... TArgs>
class thread_pool
{
private:
    std::function<void(TArgs...)> task;
    std::valarray<std::thread> do_threads;

    safe_queue<std::tuple<TArgs...>> jobs;
    std::condition_variable cond;
    std::mutex cond_mutex;

    std::atomic<bool> stopped;

public:
    thread_pool() : stopped(true) {}
    thread_pool(const thread_pool<TArgs...>& pool) = delete;
    thread_pool(thread_pool<TArgs...>&& pool) = delete;

    thread_pool<TArgs...>& operator=(const thread_pool<TArgs...>& pool) = delete;
    thread_pool<TArgs...>& operator=(thread_pool<TArgs...>&& pool) = delete;

    void start(std::size_t dojob, std::function<void(TArgs...)>&& task);
    void post(TArgs... args);
    void stop();

private:
    void do_job();
};

template <typename... TArgs>
void thread_pool<TArgs...>::start(std::size_t dojob, std::function<void(TArgs...)>&& task)
{
    if (stopped)
    {
        stopped = false;
        this->task = std::move(task);
        if (dojob < 1)
            dojob = 1;
        do_threads = std::valarray<std::thread>(dojob);
        for (std::size_t i = 0; i < dojob; i++)
        {
            do_threads[i] = std::thread(std::mem_fn(&thread_pool<TArgs...>::do_job), this);
        }
    }
}

template <typename... TArgs>
void thread_pool<TArgs...>::post(TArgs... args)
{
    jobs.emplace(args...);
    cond.notify_one();
}

template <typename... TArgs>
void thread_pool<TArgs...>::stop()
{
    if (!stopped)
    {
        stopped = true;
        cond.notify_all();
        for (std::thread& t : do_threads)
        {
            t.join();
        }
    }
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
        if (stopped)
            break;
        std::optional<std::tuple<TArgs...>> j = jobs.try_pop();
        if (j.has_value())
        {
            apply(task, j.value());
        }
    }
}
