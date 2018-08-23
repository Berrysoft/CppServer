//线程安全队列
#pragma once
#include <deque>
#include <mutex>
#include <shared_mutex>

template <typename T, typename Container = std::deque<T>>
class safe_queue
{
public:
    typedef Container container_type;
    typedef typename Container::value_type value_type;
    typedef typename Container::size_type size_type;
    typedef typename Container::reference reference;
    typedef typename Container::const_reference const_reference;
    typedef std::shared_mutex mutex_type;
    typedef std::unique_lock<std::shared_mutex> unique_lock_type;
    typedef std::shared_lock<std::shared_mutex> shared_lock_type;
protected:
    container_type container;
    mutex_type mutex;
public:
    safe_queue() : safe_queue(container_type()) {}
    explicit safe_queue(container_type &&cont) : container(cont) {}
    safe_queue(const safe_queue &queue) = delete;
    safe_queue(safe_queue &&queue) : container(std::move(queue.container)), mutex(std::move(queue.mutex)) {}
    safe_queue &operator=(const safe_queue &queue) = delete;
    safe_queue &operator=(safe_queue &&queue) 
    {
        container = std::move(queue.container);
        mutex = std::move(queue.mutex);
    }

    reference front()
    {
        shared_lock_type locker(mutex);
        return container.front();
    }
    reference back()
    {
        shared_lock_type locker(mutex);
        return container.back();
    }

    bool empty()
    {
        shared_lock_type locker(mutex);
        return container.empty();
    }
    size_type size()
    {
        shared_lock_type locker(mutex);
        return container.size();
    }

    void push(const value_type &value)
    {
        unique_lock_type locker(mutex);
        container.push_back(value);
    }
    void push(value_type &&value)
    {
        unique_lock_type locker(mutex);
        container.push_back(value);
    }

    template <typename... Args>
    decltype(auto) emplace(Args &&... args)
    {
        unique_lock_type locker(mutex);
        container.emplace_back(args...);
    }

    bool try_pop(value_type &value)
    {
        unique_lock_type locker(mutex);
        if(!container.empty())
        {
            value = std::move(container.front());
            container.pop_front();
            return true;
        }
        return false;
    }
};
