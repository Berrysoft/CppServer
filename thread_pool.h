#pragma once
#include <queue>
#include <valarray>
#include <thread>
#include <mutex>
#include <condition_variable>

template <typename T>
class thread_pool
{
private:
    void (*task)(T *);
    std::queue<T *> jobs;
    std::valarray<std::thread> do_threads;

    std::mutex m_mutex;
    std::condition_variable cond;
    std::mutex cond_mutex;
public:
    thread_pool(void (*task)(T *), std::size_t dojob = 8);
    ~thread_pool();

    void post(T *job);

private:
    static void do_job(thread_pool<T> *pool);
};

template <typename T>
thread_pool<T>::thread_pool(void (*task)(T *), std::size_t dojob) : task(task)
{
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
        jobs.push(job);
    }
    cond.notify_one();
}

template <typename T>
void thread_pool<T>::do_job(thread_pool<T> *pool)
{
    while (true)
    {
        {
            std::unique_lock<std::mutex> locker(pool->cond_mutex);
            pool->cond.wait(locker);
        }
        T *j = nullptr;
        {
            std::lock_guard<std::mutex> locker(pool->m_mutex);
            if (!pool->jobs.empty())
            {
                j = pool->jobs.front();
                pool->jobs.pop();
            }
        }
        if (!j)
            break;
        pool->task(j);
    }
}
