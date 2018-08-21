//线程安全队列
#pragma once
#include <deque>
#include <mutex>

template <typename T, typename Container = std::deque<T>>
class safe_queue
{
public:
    typedef Container container_type;
    typedef typename Container::value_type value_type;
    typedef typename Container::size_type size_type;
    typedef typename Container::reference reference;
    typedef typename Container::const_reference const_reference;
protected:
    container_type container;
    std::mutex mutex;
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
        std::lock_guard<std::mutex> locker(mutex);
        return container.front();
    }
    const_reference front() const
    {
        return container.front();
    }
    reference back()
    {
        std::lock_guard<std::mutex> locker(mutex);
        return container.back();
    }
    const_reference back() const
    {
        return container.back();
    }

    bool empty() const
    {
        return container.empty();
    }
    size_type size() const
    {
        return container.size();
    }

    void push(const value_type &value)
    {
        std::lock_guard<std::mutex> locker(mutex);
        container.push_back(value);
    }
    void push(value_type &&value)
    {
        std::lock_guard<std::mutex> locker(mutex);
        container.push_back(value);
    }

    template <typename... Args>
    decltype(auto) emplace(Args &&... args)
    {
        std::lock_guard<std::mutex> locker(mutex);
        container.emplace_back(args...);
    }

    bool try_pop(value_type &value)
    {
        std::lock_guard<std::mutex> locker(mutex);
        if(!container.empty())
        {
            value = std::move(container.front());
            container.pop_front();
            return true;
        }
        return false;
    }
};
