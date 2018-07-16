#pragma once
#include <vector>
#include <valarray>
#include <thread>
#include <mutex>
#include <condition_variable>

template <typename T>
class thread_pool
{
private:
    void (*task)(T *);
    std::vector<T *> jobs;
    std::valarray<std::thread> do_threads;

    std::mutex m_mutex;
    std::condition_variable cond;
    std::mutex cond_mutex;

    bool stop;

public:
    thread_pool(std::size_t dojob, void (*task)(T *));
    ~thread_pool();

    void post(T *job);
    void remove(T *job);

private:
    static void do_job(thread_pool<T> *pool);
};

template <typename T>
thread_pool<T>::thread_pool(std::size_t dojob, void (*task)(T *)) : task(task)
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

template <typename T>
thread_pool<T>::~thread_pool()
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

template <typename T>
void thread_pool<T>::post(T *job)
{
    {
        std::lock_guard<std::mutex> locker(m_mutex);
        jobs.push_back(job);
    }
    cond.notify_one();
}

template <typename T>
void thread_pool<T>::remove(T *job)
{
    {
        std::lock_guard<std::mutex> locker(m_mutex);
        for (std::size_t i = 0; i < jobs.size(); i++)
        {
            if (jobs[i] == job || *(jobs[i]) == *job)
            {
                jobs.erase(jobs.begin() + i);
            }
        }
    }
}

template <typename T>
void thread_pool<T>::do_job(thread_pool<T> *pool)
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
        T *j = nullptr;
        {
            std::lock_guard<std::mutex> locker(pool->m_mutex);
            if (!pool->jobs.empty())
            {
                typename std::vector<T *>::iterator it = pool->jobs.begin();
                j = *it;
                pool->jobs.erase(it);
            }
        }
        if (!j)
            continue;
        pool->task(j);
    }
}
