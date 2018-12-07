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
    
protected:
    SocketOpt() : flags_(0), status_(S_DISCONNECTED) {

    }
    
    uint16_t flags_;
    status_t status_;
};

#endif