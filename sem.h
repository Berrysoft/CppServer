#pragma once
#include <semaphore.h>

class semaphore
{
private:
    sem_t m_sem;
public:
    semaphore() :semaphore(false, 0) {}
    semaphore(bool shared, unsigned int value);
    ~semaphore();

    sem_t& sem() { return m_sem; }

    void wait();
    void trywait();
    void timedwait(const timespec* abs_timeout);
    void timedwait(int rel_sec, int rel_nsec);

    void post();
};