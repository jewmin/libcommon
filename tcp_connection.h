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

protected:
    //���ڹر��¼�֪ͨ����
    virtual void OnClosing();
    //�ѹر��¼�֪ͨ����
    virtual void OnClosed();
    //��ʱ��������
    virtual void OnTick();
    //���ӳɹ��¼�֪ͨ����
    virtual void OnConnected();
    //����ʧ���¼�֪ͨ����
    virtual void OnConnectFailed();
    //�Ͽ������¼�֪ͨ����
    virtual void OnDisconnect();
    //�ѶϿ������¼�֪ͨ����
    virtual void OnDisconnected();
    //���ݽ��մ����¼�֪ͨ����
    virtual void OnRecv(const char * data, int nread);

protected:
    TcpServer * _server;
};

#endif