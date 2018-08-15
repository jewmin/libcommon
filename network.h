#ifndef __LIB_COMMON_NETWORK_H__
#define __LIB_COMMON_NETWORK_H__

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

    inline void SetNoDelay(bool no_delay) { this->_no_delay = no_delay; }
    inline bool GetNoDelay() { return this->_no_delay; }

    inline void SetKeepAlive(uint32_t keep_alive) { this->_keep_alive = keep_alive; }
    inline uint32_t GetKeepAlive() { return this->_keep_alive; }

protected:
    explicit SocketOpt() { this->Reset(); }
    
    inline void AddFlag(flag_t flag) { this->_flags |= flag; }
    inline void RemoveFlag(flag_t flag) { this->_flags &= ~flag; }
    inline bool HasFlag(flag_t flag) { return !!(this->_flags & flag); }
    inline void ClearFlag() { this->_flags = 0; }

    inline void SetStatus(status_t status) { this->_status = status; }
    inline status_t GetStatus() { return this->_status; }

    inline void Reset()
    {
        this->_flags = 0;
        this->_status = S_DISCONNECTED;
        this->_no_delay = false;
        this->_keep_alive = 0;
    }

private:
    /*
     * No copies do not implement
     */
    SocketOpt(const SocketOpt & rhs);
    SocketOpt & operator =(const SocketOpt & rhs);


private:
    uint16_t _flags;
    status_t _status;

    uint32_t _keep_alive;
    bool _no_delay;
};

#endif