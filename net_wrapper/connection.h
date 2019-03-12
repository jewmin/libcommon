#ifndef __LIBCOMMON_NET_WRAPPER_CONNECTION_H__
#define __LIBCOMMON_NET_WRAPPER_CONNECTION_H__

#include "net_wrapper/event_pipe_impl.h"
#include "net_wrapper/event_pipe_handler.h"

namespace NetWrapper {
    class CConnectionMgr;
    class CConnection : public IEventPipeHandler {
    public:
        CConnection(CConnectionMgr * mgr, uint32_t max_out_buffer_size, uint32_t max_in_buffer_size);
        virtual ~CConnection();
        virtual void OnConnected();
        virtual void OnConnectFailed(EPipeConnFailedReason reason);
        virtual void OnDisconnect(EPipeDisconnectReason reason, bool remote);
        virtual void OnDisconnected(EPipeDisconnectReason reason, bool remote);
        virtual void OnNewDataReceived();
        virtual void OnSomeDataSent();
        virtual void OnError(int error);

        virtual void BeforeWrite(const uint8_t * data, size_t len);
        virtual void AfterWrite(size_t len);
        virtual size_t SafeWrite(const uint8_t * data, size_t len);
        virtual int Write(const uint8_t * data, size_t len);
        virtual int Send(const uint8_t * data, size_t len);

        virtual void NeedToShutdown();
        virtual void NeedToShutdown(const char * reason);
        virtual void Shutdown();
        virtual void ShutdownNow();
        virtual void OnPreAppShutDown();
        virtual std::string GetInfo();

        virtual uint8_t * ReverseGetOutputData(size_t len);
        virtual uint8_t * GetRecvData() const;
        virtual size_t GetRecvDataSize() const;
        virtual void PopRecvData(size_t len);

        void SetMaxOutBufferSize(uint32_t size);
        void SetMaxInBufferSize(uint32_t size);
        size_t GetInBufferSize();
        size_t GetOutBufferSize();
        size_t GetOutBufferUsedSize();

        void DumpOutBuf();
        void DumpInBuf();

        inline uint32_t GetConnectionID() const {
            return connection_id_;
        }

        inline void SetConnectionID(uint32_t id) {
            connection_id_ = id;
        }

        inline bool GetIsRegister2Mgr() const {
            return register_mgr_;
        }

        inline void SetIsRegister2Mgr(bool reg) {
            register_mgr_ = reg;
        }

        inline CConnectionMgr * GetMgr() const {
            return connection_mgr_;
        }

        inline void SetMgr(CConnectionMgr * mgr) {
            connection_mgr_ = mgr;
        }

        inline bool IsRawSendRecv() const {
            return raw_send_recv_;
        }

        inline void SetRawSendRecv(bool raw) {
            raw_send_recv_ = raw;
        }

        inline bool IsNeedToShutdown() const {
            return need_to_shutdown_;
        }

        inline CEventPipe * GetRealEventPipe() {
            return event_pipe_;
        }

        inline CSocket * GetSocket() {
            return event_pipe_->GetSocket();
        }
        
        inline SocketOpt::status_t GetConnectState() const {
            return event_pipe_->GetConnectState();
        }

        inline void SetConnectState(SocketOpt::status_t state) {
            event_pipe_->SetConnectState(state);
        }

        inline uint32_t GetMaxOutBufferSize() const {
            return event_pipe_->GetMaxOutBufferSize();
        }

        inline uint32_t GetMaxInBufferSize() const {
            return event_pipe_->GetMaxInBufferSize();
        }

    private:
        CConnectionMgr * connection_mgr_;
        CEventPipe * event_pipe_;
        uint32_t connection_id_;
        bool register_mgr_;
        bool raw_send_recv_;
        bool need_to_shutdown_;
    };
}

#endif