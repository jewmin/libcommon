#ifndef __LIBCOMMON_EXCEPTION_H__
#define __LIBCOMMON_EXCEPTION_H__

#include <string>

class BaseException {
public:
    BaseException(const std::string & where, const std::string & message);
    virtual ~BaseException();

    virtual std::string Where() const;
    virtual std::string Message() const;

protected:
    const std::string where_;
    const std::string message_;
};

#endif