#ifndef __UNIT_TEST_LOG_TEST_H__
#define __UNIT_TEST_LOG_TEST_H__

#include "gmock/gmock.h"
#include "log.h"
#include "mutex.h"

class MockLog : public ILog
{
    Mutex _lock;
public:
    MockLog() {}
    ~MockLog() {}

    inline const char * format_msg(const char * fmt, va_list args)
    {
        static char buf[1024];
        int ret = vsnprintf(buf, sizeof(buf), fmt, args);
        if (ret >= sizeof(buf)) buf[sizeof(buf) - 1] = 0;
        else buf[ret] = 0;
        return buf;
    }

    virtual void Info(const char * fmt, ...)
    {
        this->_lock.Lock();
        va_list args;
        va_start(args, fmt);
        const char * msg = format_msg(fmt, args);
        va_end(args);
        printf("[INFO] %s\n", msg);
        this->_lock.Unlock();
    }

    virtual void Warn(const char * fmt, ...)
    {
        this->_lock.Lock();
        va_list args;
        va_start(args, fmt);
        const char * msg = format_msg(fmt, args);
        va_end(args);
        printf("[WARN] %s\n", msg);
        this->_lock.Unlock();
    }

    virtual void Error(const char * fmt, ...)
    {
        this->_lock.Lock();
        va_list args;
        va_start(args, fmt);
        const char * msg = format_msg(fmt, args);
        va_end(args);
        printf("[ERROR] %s\n", msg);
        this->_lock.Unlock();
    }

    virtual void Debug(const char * fmt, ...)
    {
        this->_lock.Lock();
        va_list args;
        va_start(args, fmt);
        const char * msg = format_msg(fmt, args);
        va_end(args);
        printf("[DEBUG] %s\n", msg);
        this->_lock.Unlock();
    }
};

#endif