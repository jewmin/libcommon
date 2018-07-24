#include "exception.h"

BaseException::BaseException(const std::string & where, const std::string & message)
    : _where(where), _message(message)
{

}

BaseException::~BaseException()
{

}

std::string BaseException::Where() const
{
    return this->_where;
}

std::string BaseException::Message() const
{
    return this->_message;
}