#ifndef __LIBCOMMON_NET_WRAPPER_SOCKET_H__
#define __LIBCOMMON_NET_WRAPPER_SOCKET_H__

#include "net_wrapper/address.h"
#include "net_wrapper/event_reactor.h"

namespace NetWrapper {
    class CSocket {
    public:
        CSocket();
        ~CSocket();
        void Open(CEventReactor * event_reactor);
        int Shutdown();
        int ShutdownRead();
        int ShutdownWrite();
        void Close();
        void SetNoDelay();
        int Bind(const CAddress & address);
        int Bind6(const CAddress & address);
        bool GetLocalAddress(CAddress & address) const;
        bool GetRemoteAddress(CAddress & address) const;
        static void Address2sockaddr_in(struct sockaddr_in & addr, const CAddress & address);
        static void Address2sockaddr_in6(struct sockaddr_in6 & addr, const CAddress & address);
        static uv_tcp_t * CreateLibuvTcp(uv_loop_t * loop);
        static void LibuvNoDelay(uv_tcp_t * tcp);

        inline uv_tcp_t * GetLibuvTcp() const {
            return socket_;
        }

        inline bool IsShutdownRead() const {
            return is_shutdown_read_;
        }

        inline void SetShutdownRead(bool shutdown_read) {
            is_shutdown_read_ = shutdown_read;
        }

    protected:
        static void LibuvShutdownCb(uv_shutdown_t * req, int status);
        static void LibuvCloseCb(uv_handle_t * handle);

    private:
        uv_tcp_t * socket_;
        bool is_shutdown_read_;
        bool is_shutdown_write_;
    };
}

#endif