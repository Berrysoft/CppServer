#pragma once
#include <queue>
#include <valarray>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "sem.h"

template<typename T>
class thread_pool
{
private:
    void(*task)(T*);
    std::queue<T*> jobs;
    std::valarray<std::thread> receive_threads;
    std::valarray<std::thread> do_threads;

    std::mutex m_mutex;
    std::condition_variable cond;
    std::mutex cond_mutex;
    semaphore m_sem;

    std::size_t post_current;
public:
    thread_pool(void(*task)(T*), unsigned int amount = 8, std::size_t receive = 8, std::size_t dojob = 8);
    ~thread_pool();

    void post(T* job);
private:
    static void post_job(thread_pool<T>* pool, T* job);
    static void do_job(thread_pool<T>* pool);
};

template<typename T>
thread_pool<T>::thread_pool(void(*task)(T *), unsigned int amount, std::size_t receive, std::size_t dojob) :task(task)
{
    if (amount < 1)amount = 1;
    if (receive < 1)receive = 1;
    if (dojob < 1)dojob = 1;
    m_sem = semaphore(false, amount);
    receive_threads = std::valarray<std::thread>(receive);
    post_current = receive;
    do_threads = std::valarray<std::thread>(dojob);
    for (std::size_t i = 0; i < dojob; i++)
    {
        do_threads[i] = std::thread(do_job, this);
    }
}

template<typename T>
thread_pool<T>::~thread_pool()
{
    m_mutex.unlock();
    for (std::thread& t : receive_threads)
    {
        try
        {
            t.join();
        }
        catch (const std::system_error&) {}
    }
    cond.notify_all();
    for (std::thread& t : do_threads)
    {
        try
        {
            t.join();
        }
        catch (const std::system_error&) {}
    }
}

template<typename T>
void thread_pool<T>::post(T * job)
{
    post_current++;
    if (post_current >= receive_threads.size())
        post_current = 0;
    std::thread& t = receive_threads[post_current];
    try
    {
        t.join();
    }
    catch (const std::system_error&) {}
    t = std::thread(post_job, this, job);
}

template<typename T>
void thread_pool<T>::post_job(thread_pool<T>* pool, T* job)
{
    pool->m_sem.wait();
    {
        std::lock_guard<std::mutex> locker(pool->m_mutex);
        pool->jobs.push(job);
    }
    pool->m_sem.post();
    pool->cond.notify_one();
}

template<typename T>
void thread_pool<T>::do_job(thread_pool<T>* pool)
{
    while (true)
    {
        {
            std::unique_lock<std::mutex> locker(pool->cond_mutex);
            pool->cond.wait(locker);
        }
        T* j = nullptr;
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
