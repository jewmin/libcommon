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
    //�ر����Ӻ���������̳�
    virtual void Close();
    //��ʱ��������������̳�
    virtual void OnTick();
    //���ӳɹ�֪ͨ����������̳�
    virtual void OnConnected();
    //����ʧ��֪ͨ����������̳�
    virtual void OnConnectFailed();
    //�Ͽ�����֪ͨ����������̳�
    virtual void OnDisconnect();
    //�ѶϿ�����֪ͨ����������̳�
    virtual void OnDisconnected();
    //���յ����ݣ�֪ͨ������������̳�
    virtual void OnRecv(const char * data, int nread);

protected:
    TcpServer * _server;
};

#endif