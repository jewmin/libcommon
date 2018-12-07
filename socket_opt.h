#ifndef __LIBCOMMON_NETWORK_H__
#define __LIBCOMMON_NETWORK_H__

#include "uv.h"

class SocketOpt {
public:
    typedef enum {
        F_LISTEN    = 0x0001,
        F_CONNECT   = 0x0002,
        F_ACCEPT    = 0x0004,
        F_WRITING   = 0x0008,
        F_READING   = 0x0010,
        F_CLOSING   = 0x0020
    } flag_t;

    typedef enum { 
        S_CONNECTING,
        S_CONNECTED,
        S_DISCONNECTING,
        S_DISCONNECTED
    } status_t;

    inline status_t status() {
        return status_;
    }

    inline void EnableReading() {
        flags_ |= F_READING;
    }

    inline void DisableReading() {
        flags_ &= ~F_READING;
    }

    inline bool IsReading() const {
        return !!(flags_ & F_READING);
    }

    inline void EnableWriting() {
        flags_ |= F_WRITING;
    }

    inline void DisableWriting() {
        flags_ &= ~F_WRITING;
    }

    inline bool IsWriting() const {
        return !!(flags_ & F_WRITING);
    }
    
protected:
    SocketOpt() : flags_(0), status_(S_DISCONNECTED) {

    }
    
    uint16_t flags_;
    status_t status_;
};

#endif