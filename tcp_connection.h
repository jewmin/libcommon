#ifndef __LIB_COMMON_TCP_CONNECTION_H__
#define __LIB_COMMON_TCP_CONNECTION_H__

#include "tcp_service.h"
#include "tcp_server.h"

class TcpConnection : public TcpService
{
    friend class TcpService;
    friend class TcpServer;
    typedef TcpService Super;

public:
    explicit TcpConnection(TcpServer & server);
    virtual ~TcpConnection();

    inline bool operator < (const TcpConnection & right) const
    {
        if (this->_index < right._index)
            return true;
        else
            return false;
    }

protected:
    //正在关闭事件通知函数
    virtual void OnClosing();
    //已关闭事件通知函数
    virtual void OnClosed();
    //定时器处理函数
    virtual void OnTick();
    //连接成功事件通知函数
    virtual void OnConnected();
    //连接失败事件通知函数
    virtual void OnConnectFailed();
    //断开连接事件通知函数
    virtual void OnDisconnect();
    //已断开连接事件通知函数
    virtual void OnDisconnected();
    //数据接收处理事件通知函数
    virtual void OnRecv(const char * data, int nread);

protected:
    uint32_t _index;
    TcpServer * _server;
};

#endif