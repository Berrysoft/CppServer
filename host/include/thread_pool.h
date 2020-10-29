//线程池模板类。
#pragma once
#include <atomic>
#include <condition_variable>
#include <functional>
#include <mem_fn_bind.h>
#include <memory>
#include <mutex>
#include <safe_queue.h>
#include <thread>
#include <tuple>
#include <vector>

template <typename... TArgs>
class thread_pool
{
private:
    std::function<void(TArgs...)> task;

    safe_queue<std::tuple<TArgs...>> jobs;
    std::condition_variable cond;
    std::mutex cond_mutex;

    std::atomic<bool> stopped;

    std::vector<std::thread> do_threads;

public:
    thread_pool() : stopped(true) {}
    thread_pool(const thread_pool& pool) = delete;
    thread_pool(thread_pool&& pool) = delete;

    thread_pool& operator=(const thread_pool& pool) = delete;
    thread_pool& operator=(thread_pool&& pool) = delete;

    void start(std::size_t dojob, std::function<void(TArgs...)>&& task)
    {
        if (stopped.exchange(false))
        {
            this->task = std::move(task);
            if (dojob < 1)
                dojob = 1;
            do_threads.reserve(dojob);
            for (std::size_t i = 0; i < dojob; i++)
            {
                do_threads.emplace_back(mem_fn_bind(&thread_pool::do_job, this));
            }
        }
    }

    void post(TArgs... args)
    {
        jobs.emplace(args...);
        cond.notify_one();
    }

    void stop()
    {
        if (!stopped.exchange(true))
        {
            cond.notify_all();
            for (std::thread& t : do_threads)
            {
                t.join();
            }
        }
    }

    ~thread_pool() { stop(); }

private:
    void do_job()
    {
        while (!stopped)
        {
            if (jobs.empty())
            {
                std::unique_lock<std::mutex> locker(cond_mutex);
                cond.wait(locker);
            }
            std::optional<std::tuple<TArgs...>> j = jobs.try_pop();
            if (j)
            {
                std::apply(task, *j);
            }
        }
    }
};
