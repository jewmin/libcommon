#ifndef __LIB_COMMON_TCP_CLIENT_H__
#define __LIB_COMMON_TCP_CLIENT_H__

#include "tcp_service.h"

class TcpClient : public TcpService
{
    friend class TcpService;
    typedef TcpService Super;

public:
    TcpClient(const char * name, uint32_t tick, uint32_t max_out_buffer_size, uint32_t max_in_buffer_size, uint32_t reconnect_tick, ILog * logger = NULL);
    virtual ~TcpClient();

    int Connect(const char * host, uint16_t port);
    int ReConnect();

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

    //libuv�ص�������
    static void ReConnectTimerCallback(uv_timer_t * handle);

protected:
    uv_timer_t _tick_handle;
    uv_timer_t _reconnect_tick_handle;
    uv_connect_t _tcp_connect_req;

    char _name[64];
    uint32_t _tick;
    uint32_t _max_out_buffer_size;
    uint32_t _reconnect_tick;

    char _host[256];
    uint16_t _port;
};

#endif