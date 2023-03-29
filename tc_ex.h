#pragma

#include <stdexcept>
using namespace std;

namespace tars
{
/////////////////////////////////////////////////
/** 
* @file  tc_ex.h 
* @brief 异常类 
*/           
/////////////////////////////////////////////////

/**
* @brief 异常类.
*/
class TC_Exception : public exception
{
public:
    /**
     * @brief 构造函数，提供了一个可以传入errno的构造函数， 
     *  
     *        异常抛出时直接获取的错误信息
     *  
     * @param buffer 异常的告警信息 
     */
    explicit TC_Exception(const string &buffer);

    /**
     * @brief 构造函数,提供了一个可以传入errno的构造函数， 
     *  
     *        异常抛出时直接获取的错误信息
     *  
     * @param buffer 异常的告警信息 
     * @param err    错误码, 可用strerror获取错误信息
     */
    TC_Exception(const string &buffer, int err);

    /**
     * @brief 析够数函
     */
    virtual ~TC_Exception() throw();

    /**
     * @brief 错误信息.
     *
     * @return const char*
     */
    virtual const char* what() const throw();

    /**
     * @brief 获取错误码
     * 
     * @return 成功获取返回0
     */
    inline int getErrCode();
private:
    /// @brief 记录当前程序的函数调用栈信息，以便在出现异常时打印出错的位置和原因。
    void getBacktrace();

private:
    /**
     * 异常的相关信息
     */
    string  m_buffer;

    /**
     * 错误码
     */
    int     m_code;

};

}