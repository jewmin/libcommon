#ifndef __LIBCOMMON_NETWORK_H__
#define __LIBCOMMON_NETWORK_H__

#include "uv.h"

class SocketOpt
{
public:
    typedef enum
    {
        F_LISTEN    = 0x0001,
        F_CONNECT   = 0x0002,
        F_ACCEPT    = 0x0004,
        F_WRITING   = 0x0008,
        F_READING   = 0x0010,
        F_CLOSING   = 0x0020,
    } flag_t;

    typedef enum { S_CONNECTING, S_CONNECTED, S_DISCONNECTING, S_DISCONNECTED } status_t;

    inline void SetNoDelay(bool no_delay) { no_delay_ = no_delay; }
    inline bool GetNoDelay() { return no_delay_; }

    inline void SetKeepAlive(uint32_t keep_alive) { keep_alive_ = keep_alive; }
    inline uint32_t GetKeepAlive() { return keep_alive_; }

protected:
    SocketOpt() { Reset(); }
    
    inline void AddFlag(flag_t flag) { flags_ |= flag; }
    inline void RemoveFlag(flag_t flag) { flags_ &= ~flag; }
    inline bool HasFlag(flag_t flag) { return !!(flags_ & flag); }
    inline void ClearFlag() { flags_ = 0; }

    inline void SetStatus(status_t status) { status_ = status; }
    inline status_t GetStatus() { return status_; }

    inline void Reset()
    {
        flags_ = 0;
        status_ = S_DISCONNECTED;
        no_delay_ = false;
        keep_alive_ = 0;
    }

private:
    uint16_t flags_;
    status_t status_;

    uint32_t keep_alive_;
    bool no_delay_;
};

#endif