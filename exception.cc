#include "exception.h"

BaseException::BaseException(const std::string & where, const std::string & message)
    : where_(where), message_(message)
{

}

BaseException::~BaseException()
{

}

std::string BaseException::Where() const
{
    return where_;
}

std::string BaseException::Message() const
{
    return message_;
}