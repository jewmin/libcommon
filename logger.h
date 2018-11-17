#ifndef __LIBCOMMON_LOGGER_H__
#define __LIBCOMMON_LOGGER_H__

#include <time.h>
#include <stdarg.h>
#include "mutex.h"
#include "thread.h"
#include "queue.hpp"
#include "packet_pool.hpp"

class Logger : protected BaseThread {
public:
    /* Indicates the log level */
    typedef enum LogLevel {
        Trace,  /*< The trace log level */
        Debug,  /*< The debug log level */
        Info,   /*< The info log level */
        Warn,   /*< The warn log level */
        Error,  /*< The error log level */
        Fatal   /*< The fatal log level */
    } LogLevel;

    Logger();
    virtual ~Logger();

    /* Function used to initialize the logger. */
    virtual int InitLogger(LogLevel log_level);

    /* Function used to deinitialize the logger. */
    virtual void DeInitLogger();

    inline void SetLogLevel(LogLevel log_level) {
        log_level_ = log_level;
    }

#define LogTrace(fmt, ...)  LogStub(Logger::Trace, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define LogDebug(fmt, ...)  LogStub(Logger::Debug, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define LogInfo(fmt, ...)   LogStub(Logger::Info, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define LogWarn(fmt, ...)   LogStub(Logger::Warn, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define LogError(fmt, ...)  LogStub(Logger::Error, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define LogFatal(fmt, ...)  LogStub(Logger::Fatal, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
    void LogStub(LogLevel log_level, const char * file, const char * func, const int line, const char * fmt, ...);

protected:
    void LogStub2(LogLevel log_level, const char * file, const char * func, const int line, const char * fmt, va_list ap);
    const char * GetLogPrefix(const LogLevel log_level);
    void SingleRun();

    void Run() override;
    void OnTerminated() override;

    virtual void Update(struct tm * time_info);
    virtual void Log(LogLevel log_level, const char * time_string, const char * msg);

protected:
    LogLevel log_level_;
    LockQueue<Packet *> log_list_;
    Mutex allocator_lock_;
    PacketPool allocator_;
};

#endif