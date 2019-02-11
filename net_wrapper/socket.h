#ifndef __LIBCOMMON_NET_WRAPPER_SOCKET_H__
#define __LIBCOMMON_NET_WRAPPER_SOCKET_H__

#include "net_wrapper/address.h"
#include "net_wrapper/event_reactor.h"

namespace NetWrapper {
    typedef enum {
        ADDRESSFAMILY_IPV4,
        ADDRESSFAMILY_IPV6,
    } EAddressFamily;

    typedef enum {
        ADDRESSTYPE_LOCAL,
        ADDRESSTYPE_REMOTE,
    } EAddressType;

    class CSocket {
    public:
        CSocket();
        ~CSocket();
        void Open(CEventReactor * event_reactor);
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
        static void LibuvShutdownCb(uv_shutdown_t * req, int status);
        static void LibuvCloseCb(uv_handle_t * handle);

        inline uv_tcp_t * GetLibuvTcp() const {
            return tcp_;
        }

        inline void SetLibuvTcp(uv_tcp_t * tcp) {
            is_shutdown_read_ = false;
            is_shutdown_write_ = false;
            tcp_ = tcp;
        }

        inline bool IsShutdownRead() const {
            return is_shutdown_read_;
        }

        inline void SetShutdownRead(bool shutdown_read) {
            is_shutdown_read_ = shutdown_read;
        }

        inline void Shutdown() {
            if (tcp_) {
                ShutdownRead();
                ShutdownWrite();
            }
        }

        inline void ShutdownRead() {
            if (tcp_) {
                is_shutdown_read_ = true;
                int err = uv_read_stop(reinterpret_cast<uv_stream_t *>(tcp_));
                if (0 != err) {
                    OutputMsg(Logger::Error, "uv_read_stop error[%s]", uv_strerror(err));
                }
            }
        }

        inline void ShutdownWrite() {
            if (tcp_) {
                is_shutdown_write_ = true;
                uv_shutdown_t * shutdown_req = new uv_shutdown_t();
                int err = uv_shutdown(shutdown_req, reinterpret_cast<uv_stream_t *>(tcp_), LibuvShutdownCb);
                if (0 != err) {
                    OutputMsg(Logger::Error, "uv_shutdown error[%s]", uv_strerror(err));
                    delete shutdown_req;
                } else {
                    tcp_ = nullptr;
                }
            }
        }

    protected:
        int BindImpl(const CAddress & address, EAddressFamily family);
        bool GetAddressImpl(CAddress & address, EAddressType type) const;

    private:
        uv_tcp_t * tcp_;
        bool is_shutdown_read_;
        bool is_shutdown_write_;
    };
}

#endif