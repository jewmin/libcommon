#ifndef __LIBCOMMON_NET_WRAPPER_EVENT_PIPE_H__
#define __LIBCOMMON_NET_WRAPPER_EVENT_PIPE_H__

#include <vector>
#include <iostream>
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
        class CContext {
        public:
            CContext(const std::shared_ptr<CEventPipe> & event_pipe);
            ~CContext();
            std::weak_ptr<CEventPipe> event_pipe_;
        };

        class WriteRequest {
        public:
            WriteRequest(const std::shared_ptr<CEventPipe> & event_pipe, const char * data, size_t size);
            ~WriteRequest();
            std::weak_ptr<CEventPipe> event_pipe_;
            std::vector<char> storage_;
            uv_write_t req_;
            uv_buf_t buf_;
        };

        class CIOBuffer {
        public:
            CIOBuffer(CEventPipe * event_pipe);
            ~CIOBuffer();
            void InitWriteRequest();

            inline uv_write_t * GetLibuvWriteReq() {
                return &request_->req_;
            }

            inline uv_buf_t * GetLibuvBuf() {
                return &request_->buf_;
            }

            CEventPipe * event_pipe_;
            Packet * input_;
            Packet * output_;
            WriteRequest * request_;
        };

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
        virtual void CancelWrite(size_t len);
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
        void HandleError(int error);
        void SetMaxOutBufferSize(uint32_t size);
        void SetMaxInBufferSize(uint32_t size);
        size_t GetInBufferSize();
        size_t GetOutBufferSize();
        size_t GetOutBufferUsedSize();

        void AddTimer(uint32_t interval);
        void DelTimer();
        void DumpOutBuf();
        void DumpInBuf();

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

    protected:
        static void Dump(uint8_t * data, size_t len);
        static void LibuvTimerCb(uv_timer_t * handle);
        static void LibuvTimerCloseCb(uv_handle_t * handle);
        static void LibuvAllocCb(uv_handle_t * handle, size_t suggested_size, uv_buf_t * buf);
        static void LibuvReadCb(uv_stream_t * stream, ssize_t nread, const uv_buf_t * buf);
        static void LibuvWriteCb(uv_write_t * req, int status);

    private:
        CSocket socket_;
        uv_timer_t * delay_shutdown_timer_;
        CIOBuffer * buffer_;
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