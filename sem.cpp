#include "sem.h"
#include <ctime>

semaphore::semaphore(bool shared, unsigned int value)
{
    sem_init(&m_sem, shared ? 1 : 0, value);
}

semaphore::~semaphore()
{
    sem_destroy(&m_sem);
}

void semaphore::wait()
{
    sem_wait(&m_sem);
}

void semaphore::trywait()
{
    sem_trywait(&m_sem);
}

void semaphore::timedwait(const timespec * abs_timeout)
{
    sem_timedwait(&m_sem, abs_timeout);
}

void semaphore::timedwait(int rel_sec, int rel_nsec)
{
    timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += rel_sec;
    ts.tv_nsec += rel_nsec;
    timedwait(&ts);
}

void semaphore::post()
{
    sem_post(&m_sem);
}
