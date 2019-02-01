#ifndef __LIBCOMMON_NET_WRAPPER_EVENT_PIPE_H__
#define __LIBCOMMON_NET_WRAPPER_EVENT_PIPE_H__

#include "uv.h"
#include "packet.hpp"
#include "socket_opt.h"
#include "net_wrapper/socket.h"
#include "net_wrapper/reactor_handler.h"

namespace NetWrapper {
    typedef enum {
        DISCONNECTREASON_EOF,
        DISCONNECTREASON_ERROR,
    } EPipeDisconnectReason;

    typedef enum {
        CONNFAILEDREASON_ENETUNREACH,
        CONNFAILEDREASON_EADDRNOTAVAIL,
        CONNFAILEDREASON_ECONNREFUSED,
        CONNFAILEDREASON_ECONNRESET,
        CONNFAILEDREASON_EADDRINUSE,
        CONNFAILEDREASON_ETIMEDOUT,
        CONNFAILEDREASON_UNKNOWN,
        CONNFAILEDREASON_ECANCELED,
    } EPipeConnFailedReason;

    class CEventPipe : public CReactorHandler {
    public:
        CEventPipe(uint32_t max_out_buffer_size, uint32_t max_in_buffer_size);
        virtual ~CEventPipe();
        bool RegisterToReactor() override;
        bool UnRegisterFromReactor() override;
        virtual void OnConnected() {}
        virtual void OnConnectFailed(EPipeConnFailedReason reason) {}
        virtual void OnDisconnect(EPipeDisconnectReason reason, bool remote) {}
        virtual void OnDisconnected(EPipeDisconnectReason reason, bool remote) {}
        virtual void OnNewDataReceived() {}
        virtual void OnSomeDataSent() {}
        virtual void OnError(int error) {}
        virtual void HandleClose4EOF(EPipeDisconnectReason reason, bool remote);
        virtual void HandleClose4Error(EPipeDisconnectReason reason, bool remote);
        virtual int Write(const uint8_t * data, size_t len);
        virtual uint8_t * ReverseGetOutputData(size_t len);
        virtual uint8_t * GetRecvData() const;
        virtual size_t GetRecvDataSize() const;
        virtual void PopRecvData(size_t len);
        virtual void ClearInBuf();
        virtual void ClearOutBuf();

        bool Open();
        void Shutdown(bool now);
        void ShutdownImmediately();
        void CallOnDisconnected(bool remote);
        int Read(void*, unsigned long);
        void SetMaxOutBufferSize(uint32_t size);
        void SetMaxInBufferSize(uint32_t size);
        unsigned long GetInBufferSize();
        unsigned long GetOutBufferSize();
        unsigned long GetOutBufferUsedSize();

        void AddTimer(unsigned int);
        void DelTimer(); void CancelWrite(unsigned long);
        void DumpOutBuf();
        void DumpInBuf();
        static void Dump(unsigned char*, unsigned int);
        static void OnReadInternal(bufferevent*, void*);
        static void OnWriteInternal(bufferevent*, void*);
        static void OnErrorInternal(bufferevent*, short, void*);
        static void OnEventTick(int, short, void*);

        inline CSocket * GetSocket() {
            return &socket_;
        }

        inline uint32_t GetMaxOutBufferSize() const {
            return max_out_buffer_size_;
        }

        inline uint32_t GetMaxInBufferSize() const {
            return max_in_buffer_size_;
        }

        inline SocketOpt::status_t GetConnectState() const {
            return connect_state_;
        }

        inline void SetConnectState(SocketOpt::status_t state) {
            connect_state_ = state;
        }

        inline uv_connect_t * GetConnectReq() {
            return connect_req_;
        }
        
        inline void SetConnectReq(uv_connect_t * req) {
            connect_req_ = req;
        }

    protected:
        static void LibuvTimerCloseCb(uv_handle_t * handle);

    private:
        CSocket socket_;
        uv_connect_t * connect_req_;
        uv_timer_t * delay_shutdown_timer_;
        Packet * input_;
        Packet * output_;
        SocketOpt::status_t connect_state_;
        uint32_t max_out_buffer_size_;
        uint32_t max_in_buffer_size_;
        bool reg_timer_;
        bool shutdown_;
        bool called_on_disconnect_;
        bool called_on_disconnected_;
    };
}

#endif