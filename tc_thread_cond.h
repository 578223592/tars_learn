#ifndef _TC_THREAD_COND_H
#define _TC_THREAD_COND_H

#include <sys/time.h>
#include <cerrno>
#include <iostream>
#include <assert.h>
#include <pthread.h>

using namespace std;

namespace tars
{

    // 前置声明
    class TC_ThreadMutex;

    class TC_ThreadCond
    {
    public:
        TC_ThreadCond();

        ~TC_ThreadCond();

        void signal();

        void broadcast();

        timespec abstime(int millsecond) const;

        template <typename Mutex>
        void wait(const Mutex &mutex) const;

        template <typename Mutex>
        bool timedWait(const Mutex &mutex, int millsecond) const;

    protected:
        // Not implemented; prevents accidental use.
        TC_ThreadCond(const TC_ThreadCond &);
        TC_ThreadCond &operator=(const TC_ThreadCond &);

    private:
        // mutable pthread_cond_t m_cond;
        pthread_cond_t m_cond;
    };


// 普通函数和模板函数在编译期间的处理方式不同，这导致了它们在定义和声明时的行为也有所不同。
// 普通函数的定义并不会被复制到每个包含该函数的源文件中，在编译时，编译器只需要在链接阶段将所有引用该函数的目标文件中的函数引用链接到函数的定义即可。因此，如果你在多个源文件中包含同一个头文件，其中定义了普通函数，则编译器只会在生成可执行文件时链接一次该函数的定义，避免了重复定义的问题。
// 而模板函数在使用时会根据传入的参数类型生成对应的函数实例，因此，编译器需要在每个使用该模板的源文件中都生成一份模板函数实例代码，然后在链接阶段将所有模板函数实例链接起来。如果模板函数的定义和声明分别放在不同的源文件中，则编译器在生成模板函数的实例代码时无法找到模板函数的定义，从而出现编译错误。
// 综上所述，模板函数需要在定义时就提供函数实例化的代码，因此必须放在头文件中，而普通函数则不需要在定义时提供函数实例化的代码，因此可以放在源文件中。

    template <typename Mutex>
    inline void TC_ThreadCond::wait(const Mutex &mutex) const
    {
        int c = mutex.count();
        int rc = pthread_cond_wait(&_cond, &mutex._mutex);
        mutex.count(c);
        if (rc != 0)
        {
            cout << "[TC_ThreadCond::wait] pthread_cond_wait error" << endl;
        }
    }

    template <typename Mutex>
    inline bool TC_ThreadCond::timedWait(const Mutex &mutex, int millsecond) const
    {

        int c = mutex.count();

        timespec ts = abstime(millsecond);

        int rc = pthread_cond_timedwait(&_cond, &mutex._mutex, &ts);

        mutex.count(c);

        if (rc != 0)
        {
            if (rc != ETIMEDOUT)
            {
                cout << "[TC_ThreadCond::timedWait] pthread_cond_timedwait error" << endl;
            }

            return false;
        }
        return true;
    }
}

#endif
