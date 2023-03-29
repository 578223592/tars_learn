#include "tc_thread_mutex.h"

tars::TC_ThreadMutex::TC_ThreadMutex()
{
    m_mutex = PTHREAD_MUTEX_INITIALIZER;
}

tars::TC_ThreadMutex::~TC_ThreadMutex()
{
    int rc = pthread_mutex_lock(&m_mutex);
    if(rc != 0)
    {
        if(rc == EDEADLK)
        {
            cout<<"[TC_ThreadMutex::lock] pthread_mutex_lock dead lock error"<<rc<<endl;
        }
        else
        {
            cout<<"[TC_ThreadMutex::lock] pthread_mutex_lock error"<<rc<<endl;
        }
    }
}


void tars::TC_ThreadMutex::lock() const
{
    int rc = pthread_mutex_lock(&m_mutex);
    if(rc != 0)
    {
        if(rc == EDEADLK)
        {
            cout<<"[TC_ThreadMutex::lock] pthread_mutex_lock dead lock error"<<rc<<endl;
        }
        else
        {
            cout<<"[TC_ThreadMutex::lock] pthread_mutex_lock error"<<rc<<endl;
        }
    }
}

bool tars::TC_ThreadMutex::tryLock() const
{
    int rc = pthread_mutex_trylock(&m_mutex);
    if(rc != 0 && rc != EBUSY)
    {
        if(rc == EDEADLK)
        {
            cout<<"[TC_ThreadMutex::tryLock] pthread_mutex_trylock dead lock error"<<rc<<endl;
        }
        else
        {
            cout<<"[TC_ThreadMutex::tryLock] pthread_mutex_trylock error"<<rc<<endl;
        }
    }
    return (rc == 0);
}

void tars::TC_ThreadMutex::unlock() const
{
    int rc = pthread_mutex_unlock(&m_mutex);
    if(rc != 0)
    {
        cout<<"[TC_ThreadMutex::unlock] pthread_mutex_unlock error"<<rc<<endl;
    }
}

int tars::TC_ThreadMutex::count() const
{
    return 0;
}

void tars::TC_ThreadMutex::count(int c) const
{
}
