#ifndef __TEST_LOG_H__
#define __TEST_LOG_H__

#include <string>
#include <stdint.h>
#include <fstream>
#include "log.h"
#include "buffer.h"
#include "thread.h"
#include "common.h"
#include "container.h"

class TestLog : public ILog, protected BaseThread, private Buffer::Allocator
{
public:
    TestLog(const char * filename)
        : Buffer::Allocator(1500, 0), _log_filename(filename)
    {

    }

    using BaseThread::Start;
    using BaseThread::Stop;

    virtual ~TestLog()
    {
        this->_log_queue.Flush();
        size_t count = this->_log_queue.size();
        for (size_t i = 0; i < count; i++)
        {
            this->_log_queue[i]->Release();
        }
        this->_log_queue.clear();
    }

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
        va_list args;
        va_start(args, fmt);
        const char * msg = format_msg(fmt, args);
        va_end(args);

        Buffer * buffer = this->Allocate();
        buffer->AddData("[INFO] ", 7);
        buffer->AddData(msg, strlen(msg));
        buffer->AddData(0);
        this->_log_queue.Push(buffer);
    }

    virtual void Warn(const char * fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        const char * msg = format_msg(fmt, args);
        va_end(args);

        Buffer * buffer = this->Allocate();
        buffer->AddData("[WARN] ", 7);
        buffer->AddData(msg, strlen(msg));
        buffer->AddData(0);
        this->_log_queue.Push(buffer);
    }

    virtual void Error(const char * fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        const char * msg = format_msg(fmt, args);
        va_end(args);

        Buffer * buffer = this->Allocate();
        buffer->AddData("[ERROR] ", 8);
        buffer->AddData(msg, strlen(msg));
        buffer->AddData(0);
        this->_log_queue.Push(buffer);
    }

    virtual void Debug(const char * fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        const char * msg = format_msg(fmt, args);
        va_end(args);

        Buffer * buffer = this->Allocate();
        buffer->AddData("[DEBUG] ", 8);
        buffer->AddData(msg, strlen(msg));
        buffer->AddData(0);
        this->_log_queue.Push(buffer);
    }

    void Run()
    {
        while (!this->IsTerminated())
        {
            this->_log_queue.Flush();
            size_t count = this->_log_queue.size();
            for (size_t i = 0; i < count; i++)
            {
                const char * msg = (const char *)this->_log_queue[i]->GetBuffer();
                printf("%s\n", msg);
                if (!this->_log_stream.is_open())
                {
                    this->_log_stream.open(this->_log_filename, std::ios_base::out | std::ios_base::app);
                    this->_log_stream << "*********** newest log ***********" << '\n';
                }
                this->_log_stream << msg << '\n';
                this->_log_queue[i]->Release();
            }
            this->_log_queue.clear();
            jc_sleep(1);
        }
    }

private:
    LockQueue<Buffer *> _log_queue;
    std::fstream _log_stream;
    std::string _log_filename;
};

std::string ToHex(uint8_t c);

std::string DumpData(const uint8_t * const data, size_t data_length, size_t line_length = 0);

#endif