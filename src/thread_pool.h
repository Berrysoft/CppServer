//线程池模板类。
#pragma once
#include <vector>
#include <valarray>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <tuple>
#include <memory>
#include <functional>

template <typename... TArgs>
class thread_pool
{
private:
    std::function<void(TArgs...)> task;
    std::vector<std::unique_ptr<std::tuple<TArgs...>>> jobs;
    std::valarray<std::thread> do_threads;

    std::mutex m_mutex;
    std::condition_variable cond;
    std::mutex cond_mutex;

    std::atomic<bool> stop;
public:
    thread_pool(std::size_t dojob, std::function<void(TArgs...)> task);
    ~thread_pool();

    void post(TArgs... args);
    void broadcast();
private:
    static void do_job(thread_pool<TArgs...> *pool);
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
        do_threads[i] = std::thread(do_job, this);
    }
}

template <typename... TArgs>
thread_pool<TArgs...>::~thread_pool()
{
    stop = true;
    cond.notify_all();
    for (std::thread &t : do_threads)
    {
        try
        {
            t.join();
        }
        catch (const std::system_error &)
        {
        }
    }
}

template <typename... TArgs>
void thread_pool<TArgs...>::post(TArgs... args)
{
    {
        std::lock_guard<std::mutex> locker(m_mutex);
        jobs.push_back(std::make_unique<std::tuple<TArgs...>>(args...));
    }
    cond.notify_one();
}

template <typename... TArgs>
void thread_pool<TArgs...>::broadcast()
{
    cond.notify_all();
}

template <typename... TArgs>
void thread_pool<TArgs...>::do_job(thread_pool<TArgs...> *pool)
{
    while (true)
    {
        if (pool->jobs.empty())
        {
            std::unique_lock<std::mutex> locker(pool->cond_mutex);
            pool->cond.wait(locker);
        }
        if (pool->stop)
            break;
        std::unique_ptr<std::tuple<TArgs...>> j;
        {
            std::lock_guard<std::mutex> locker(pool->m_mutex);
            if (!pool->jobs.empty())
            {
                typename std::vector<std::unique_ptr<std::tuple<TArgs...>>>::iterator it = pool->jobs.begin();
                j = std::move(*it);
                pool->jobs.erase(it);
            }
        }
        if (j)
        {
            apply(pool->task, *j);
        }
    }
}
