#include "tc_thread_cond.h"
#include <string.h>

tars::TC_ThreadCond::TC_ThreadCond()
{

    // 这将对 mycond 进行静态初始化，它的属性将被设置为默认值。如果需要更复杂的属性，如动态分配条件变量的属性，则必须使用类似于构造函数中使用的 pthread_condattr_t 结构体进行初始化。
    // 跟1.3不一样，其是用attr初始化的
    m_cond = PTHREAD_COND_INITIALIZER;
}

tars::TC_ThreadCond::~TC_ThreadCond()
{
    int rc = 0;
    rc = pthread_cond_destroy(&m_cond);
    if (rc != 0)
    {
        cout << "[TC_ThreadCond::~TC_ThreadCond] pthread_cond_destroy error:" << string(strerror(rc)) << endl;
    }
}

void tars::TC_ThreadCond::signal()
{
    int rc = pthread_cond_signal(&m_cond);
    if (rc != 0)
    {
        cout << "[TC_ThreadCond::signal] pthread_cond_signal error" << endl;
    }
}
void tars::TC_ThreadCond::broadcast()
{
    int rc = pthread_cond_broadcast(&m_cond);
    if (rc != 0)
    {
        cout << "[TC_ThreadCond::broadcast] pthread_cond_broadcast error" << endl;
    }
}
// tv_sec millsecond tv_usec：秒 毫秒 微秒
timespec tars::TC_ThreadCond::abstime(int millsecond) const
{
    struct timeval tv;

    gettimeofday(&tv, 0);
    // TC_TimeProvider::getInstance()->getNow(&tv);

    int64_t it = tv.tv_sec * (int64_t)1000000 + tv.tv_usec + (int64_t)millsecond * 1000;

    tv.tv_sec = it / (int64_t)1000000;
    tv.tv_usec = it % (int64_t)1000000;

    timespec ts;
    ts.tv_sec = tv.tv_sec;
    ts.tv_nsec = tv.tv_usec * 1000;

    return ts;
}
