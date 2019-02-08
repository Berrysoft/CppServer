//线程安全队列
#pragma once
#include <deque>
#include <mutex>
#include <optional>
#include <shared_mutex>

template <typename T, typename Container = std::deque<T>, typename Mutex = std::shared_mutex>
class safe_queue
{
public:
    typedef Container container_type;
    typedef typename Container::value_type value_type;
    typedef typename Container::size_type size_type;
    typedef typename Container::reference reference;
    typedef typename Container::const_reference const_reference;
    typedef Mutex mutex_type;
    typedef std::unique_lock<Mutex> unique_lock_type;
    typedef std::shared_lock<Mutex> shared_lock_type;

protected:
    container_type container;
    mutable mutex_type mutex;

public:
    safe_queue() : safe_queue(container_type()) {}
    explicit safe_queue(container_type&& cont) : container(cont) {}
    safe_queue(const safe_queue& queue) = delete;
    safe_queue(safe_queue&& queue) = delete;
    safe_queue& operator=(const safe_queue& queue) = delete;
    safe_queue& operator=(safe_queue&& queue) = delete;

    reference front() const
    {
        shared_lock_type locker(mutex);
        return container.front();
    }
    reference back() const
    {
        shared_lock_type locker(mutex);
        return container.back();
    }

    bool empty() const
    {
        shared_lock_type locker(mutex);
        return container.empty();
    }
    size_type size() const
    {
        shared_lock_type locker(mutex);
        return container.size();
    }

    void push(const value_type& value)
    {
        unique_lock_type locker(mutex);
        container.push_back(value);
    }
    void push(value_type&& value)
    {
        unique_lock_type locker(mutex);
        container.push_back(value);
    }

    template <typename... Args>
    decltype(auto) emplace(Args&&... args)
    {
        unique_lock_type locker(mutex);
        container.emplace_back(args...);
    }

    std::optional<value_type> try_pop()
    {
        unique_lock_type locker(mutex);
        if (!container.empty())
        {
            std::optional<value_type> result = std::make_optional(std::move(container.front()));
            container.pop_front();
            return result;
        }
        return std::nullopt;
    }
};
