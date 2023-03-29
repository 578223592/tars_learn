#pragma

#include "tc_thread_mutex.h"
#include "tc_thread_cond.h"

// 模板类中的函数定义需要被包含在头文件中，因为模板类的实例化是在编译时进行的。
// 当一个使用模板类的源文件被编译时，编译器需要访问模板类的完整定义来生成适当的代码。
// 如果函数定义被放在源文件中，则编译器无法在编译时确定所有可能的模板实例，并且在链接时会出现未定义符号错误。
// 因此，在头文件中定义和实现模板类的函数是一种常见的做法。
namespace tars
{

    template <class T, class P>
    class TC_Monitor
    {
    public:
        typedef TC_LockT<TC_Monitor<T, P>> Lock;
        typedef TC_TryLockT<TC_Monitor<T, P>> TryLock;

        TC_Monitor();

        virtual ~TC_Monitor()
        {
        }

        void lock() const
        {
            m_mutex.lock();
            m_nnotify = 0;
        }

        void unlock() const
        {
            notifyImpl(m_nnotify);
            m_mutex.unlock();
        }

        bool tryLock() const
        {
            bool result = m_mutex.tryLock();
            if (result)
            {
                m_nnotify = 0;
            }
            return result;
        }

        void wait() const
        {
            cout << "TC_Monitor wait" << endl;
            notifyImpl(m_nnotify);

            try
            {
                m_cond.wait(m_mutex);
            }
            catch (...)
            {
                m_nnotify = 0;
                throw;
            }

            m_nnotify = 0;
        }

        bool timedWait(int millsecond) const
        {
            notifyImpl(m_nnotify);

            bool rc;

            try
            {
                rc = m_cond.timedWait(_mutex, millsecond);
            }
            catch (...)
            {
                m_nnotify = 0;
                throw;
            }

            m_nnotify = 0;
            return rc;
        }

        void notify()
        {
            if (m_nnotify != -1)
            {
                ++m_nnotify;
            }
        }

        void notifyAll()
        {
            m_nnotify = -1;
        }

    protected:
        void notifyImpl(int nnotify) const
        {
            if (nnotify != 0)
            {
                if (nnotify == -1)
                {
                    m_cond.broadcast();
                    return;
                }
                else
                {
                    while (nnotify > 0)
                    {
                        m_cond.signal();
                        --nnotify;
                    }
                }
            }
        }

    private:
        TC_Monitor(const TC_Monitor &);
        void operator=(const TC_Monitor &);

    protected:
        mutable int m_nnotify;
        mutable P m_cond;
        T m_mutex;
    };

    typedef TC_Monitor<TC_ThreadMutex, TC_ThreadCond> TC_ThreadLock;

    template <class T, class P>
    inline TC_Monitor<T, P>::TC_Monitor()
    {
    }

}
