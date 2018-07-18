#ifndef __LIB_COMMON_TCP_SERVICE_H__
#define __LIB_COMMON_TCP_SERVICE_H__

#include "service.h"

class TcpService : public BaseService
{
    typedef BaseService Super;

public:
    explicit TcpService(uint32_t max_in_buffer_size, ILog * logger = NULL);
    virtual ~TcpService();

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
    static void TimerCallback(uv_timer_t * handle);
    static void ConnectionCallback(uv_stream_t * server, int status);
    static void CloseCallback(uv_handle_t * handle);
    static void ConnectCallback(uv_connect_t * req, int status);
    static void AllocBufferCallback(uv_handle_t * handle, size_t suggested_size, uv_buf_t * buf);
    static void ReadCallback(uv_stream_t * stream, ssize_t nread, const uv_buf_t * buf);

protected:
    union {
        uv_handle_t handle;
        uv_stream_t stream;
        uv_tcp_t tcp;
    } _handle;

    uint32_t _max_in_buffer_size;
};

#endif