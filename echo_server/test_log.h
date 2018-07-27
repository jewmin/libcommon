#ifndef __TEST_LOG_H__
#define __TEST_LOG_H__

#include "log.h"
#include "lock_queue.h"
#include "buffer.h"
#include "thread.h"
#include <string>
#include "stdint.h"

class TestLog : public ILog, protected BaseThread, private Buffer::Allocator
{
public:
    TestLog()
        : Buffer::Allocator(1500, 0)
    {

    }

    virtual ~TestLog()
    {
        this->_log_queue.Flush();
        size_t count = this->_log_queue.size();
        for (size_t i = 0; i < count; i++)
        {

        }
        this->_log_queue.clear();
    }

    virtual void Info(const char * fmt, ...)
    {

    }

    virtual void Warn(const char * fmt, ...)
    {

    }

    virtual void Error(const char * fmt, ...)
    {

    }

    virtual void Debug(const char * fmt, ...)
    {

    }

    void Run()
    {
        while (!this->IsTerminated())
        {
            this->_log_queue.Flush();
            size_t count = this->_log_queue.size();
            for (size_t i = 0; i < count; i++)
            {

            }
            this->_log_queue.clear();
            Sleep(1);
        }
    }

private:
    LockQueue<Buffer *> _log_queue;
};

std::string DumpData(const uint8_t * const data, size_t data_length, size_t line_length = 0)
{
    return "";
}

#endif