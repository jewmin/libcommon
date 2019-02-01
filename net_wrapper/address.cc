#include "net_wrapper/address.h"

NetWrapper::CAddress::CAddress()
    : port_(0) {
    
}

NetWrapper::CAddress::CAddress(const char * ip, uint16_t port)
    : ip_(ip), port_(port) {

}

NetWrapper::CAddress::~CAddress() {

}

NetWrapper::CAddress & NetWrapper::CAddress::operator=(const CAddress & rhs) {
    ip_ = rhs.ip_;
    port_ = rhs.port_;
    return *this;
}

std::string NetWrapper::CAddress::ToString() const {
    std::stringstream ss;
    ss << ip_ << ":" << port_;
    return ss.str();
}