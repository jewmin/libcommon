#ifndef __LIB_COMMON_TCP_CONNECTION_H__
#define __LIB_COMMON_TCP_CONNECTION_H__

#include "tcp_service.h"
#include "tcp_server.h"

class TcpConnection : public TcpService
{
    friend class TcpService;
    typedef TcpService Super;

public:
    TcpConnection(TcpServer & server);
    virtual ~TcpConnection();

protected:
    //关闭连接函数，子类继承
    virtual void Close();
    //定时器处理函数，子类继承
    virtual void OnTick();
    //连接成功通知函数，子类继承
    virtual void OnConnected();
    //连接失败通知函数，子类继承
    virtual void OnConnectFailed();
    //断开连接通知函数，子类继承
    virtual void OnDisconnect();
    //已断开连接通知函数，子类继承
    virtual void OnDisconnected();
    //接收到数据，通知处理函数，子类继承
    virtual void OnRecv(const char * data, int nread);

protected:
    TcpServer * _server;
};

#endif