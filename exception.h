#ifndef __LIB_COMMON_EXCEPTION_H__
#define __LIB_COMMON_EXCEPTION_H__

#include <string>

class BaseException
{
public:
    BaseException(const std::string & where, const std::string & message);
    virtual ~BaseException();

    virtual std::string Where() const;
    virtual std::string Message() const;

protected:
    const std::string _where;
    const std::string _message;
};

#endif