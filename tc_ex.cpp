#include "tc_ex.h"
#include "tc_ex.h"
#include <execinfo.h>
#include <string.h>
#include <stdlib.h>
#include <cerrno>
namespace tars
{
    inline int tars::TC_Exception::getErrCode()
    {
        return m_code;
    }

    TC_Exception::TC_Exception(const string &buffer)
        : m_buffer(buffer), m_code(0)
    {
        //    getBacktrace();
    }

    TC_Exception::TC_Exception(const string &buffer, int err)
    {
        m_buffer = buffer + " :" + strerror(err);
        m_code = err;
        //    getBacktrace();
    }

    TC_Exception::~TC_Exception() throw()
    {
    }

    const char *TC_Exception::what() const throw()
    {
        return m_buffer.c_str();
    }

    void TC_Exception::getBacktrace()
    {
        void *array[64];
        int nSize = backtrace(array, 64);
        char **symbols = backtrace_symbols(array, nSize);

        for (int i = 0; i < nSize; i++)
        {
            m_buffer += symbols[i];
            m_buffer += "\n";
        }
        free(symbols);
    }
}