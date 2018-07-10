#ifndef __LIB_COMMON_LOG_H__
#define __LIB_COMMON_LOG_H__

class ILog
{
public:
    virtual void Info(const char * fmt, ...) = 0;
    virtual void Warn(const char * fmt, ...) = 0;
    virtual void Error(const char * fmt, ...) = 0;
    virtual void Debug(const char * fmt, ...) = 0;
};

#endif