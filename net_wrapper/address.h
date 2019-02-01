#ifndef __LIBCOMMON_NET_WRAPPER_ADDRESS_H__
#define __LIBCOMMON_NET_WRAPPER_ADDRESS_H__

#include <string>
#include <sstream>
#include "common.h"

namespace NetWrapper {
    class CAddress {
    public:
        CAddress();
        CAddress(const char * ip, uint16_t port);
        ~CAddress();
        CAddress & operator=(const CAddress & rhs);
        std::string ToString() const;

        inline const char * GetAddress() const {
            return ip_.c_str();
        }

        inline void SetAddress(const char * ip) {
            ip_ = ip;
        }

        inline uint16_t GetPort() const {
            return port_;
        }

        inline void SetPort(uint16_t port) {
            port_ = port;
        }

    private:
        std::string ip_;
        uint16_t port_;
    };
}

#endif