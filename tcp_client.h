#ifndef __LIBCOMMON_TCP_CLIENT_H__
#define __LIBCOMMON_TCP_CLIENT_H__

#include "uv.h"
#include "mutex.h"
#include "thread.h"
#include "logger.h"
#include "buffer.h"
#include "network.h"
#include "queue.hpp"

class TcpClient : public SocketOpt, protected BaseThread, protected Buffer::Allocator {
public:
    int ConnectTo(const char * host, uint16_t port);
    void WaitForShutdownToComplete();

protected:
    TcpClient(size_t max_free_buffers, size_t buffer_size = 1024, Logger * logger = nullptr);
    virtual ~TcpClient();

    uv_tcp_t * uv_tcp() {
        return &tcp_;
    }
    uv_stream_t * uv_stream() {
        return reinterpret_cast<uv_stream_t *>(&tcp_);
    }
    uv_handle_t * uv_handle() {
        return reinterpret_cast<uv_handle_t *>(&tcp_);
    }

    void Run() override;
    void OnTerminated() override;

    // ����
    int ConnectToServer();
    // �Ͽ�����
    void Shutdown();
    // �ر�����
    void Close();
    // �������ݵ�������
    bool SendData(const char * data, size_t length);
    // �ύ����
    void Send(const char * data, size_t length);
    // �����첽��Ϣ
    void AppendMessage(action_t action);

    // ���ӳɹ��¼�
    virtual void OnConnected();
    // ����ʧ���¼�
    virtual void OnConnectFailed();
    // �Ͽ������¼�
    virtual void OnDisconnected();
    // ��ȡ����¼����������������ö�ȡ������
    virtual void OnReadComplete(Buffer * buffer) = 0;
    // ��������¼��������ѷ��͵����ݳ��ȣ�<0Ϊ������
    virtual void OnWriteComplete(int written) = 0;

    static void ThreadReqCb(uv_async_t * handle);
    static void AfterConnect(uv_connect_t * req, int status);
    static void AfterClose(uv_handle_t * handle);
    static void AllocBuffer(uv_handle_t * handle, size_t suggested_size, uv_buf_t * buf);
    static void AfterRead(uv_stream_t * stream, ssize_t nread, const uv_buf_t * buf);
    static void AfterWrite(uv_write_t * req, int status);

private:
    char host_[256];        // ������
    uint16_t port_;         // �˿�

    Logger * logger_;
    state_t state_;
    Buffer * recv_buffer_;
    Buffer * send_buffer_;
    Mutex send_buffer_lock_;
    size_t send_buffer_len_;
    LockQueue<action_t> outgoing_message_queue_;

    uv_loop_t loop_;
    uv_tcp_t tcp_;
    uv_connect_t connect_req_;
    uv_write_t write_req_;
    uv_async_t thread_req_;
    uv_sem_t thread_start_sem_;
};

#endif