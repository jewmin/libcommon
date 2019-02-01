#include "net_wrapper/socket.h"

NetWrapper::CSocket::CSocket()
    : socket_(nullptr), is_shutdown_read_(true), is_shutdown_write_(true) {

}

NetWrapper::CSocket::~CSocket() {
    Close();
}

void NetWrapper::CSocket::Open(CEventReactor * event_reactor) {
    if (socket_) {
        throw BaseException(__func__, "socket_ != nullptr");
    }
    socket_ = CreateLibuvTcp(event_reactor->GetLibuvLoop());
    is_shutdown_read_ = false;
    is_shutdown_write_ = false;
}

int NetWrapper::CSocket::Shutdown() {
    if (socket_) {
        ShutdownRead();
        ShutdownWrite();
    }
    return 0;
}

int NetWrapper::CSocket::ShutdownRead() {
    if (socket_) {
        is_shutdown_read_ = true;
        int err = uv_read_stop(reinterpret_cast<uv_stream_t *>(socket_));
        if (0 != err) {
            OutputMsg(Logger::Error, "uv_read_stop error[%s]", uv_strerror(err));
        }
        return err;
    }
    return 0;
}

int NetWrapper::CSocket::ShutdownWrite() {
    if (socket_) {
        is_shutdown_write_ = true;
        uv_shutdown_t * shutdown_req = new uv_shutdown_t();
        int err = uv_shutdown(shutdown_req, reinterpret_cast<uv_stream_t *>(socket_), LibuvShutdownCb);
        if (0 != err) {
            OutputMsg(Logger::Error, "uv_shutdown error[%s]", uv_strerror(err));
            delete shutdown_req;
        } else {
            socket_ = nullptr;
        }
        return err;
    }
    return 0;
}

void NetWrapper::CSocket::Close() {
    if (socket_) {
        if (!uv_is_closing(reinterpret_cast<uv_handle_t *>(socket_))) {
            uv_close(reinterpret_cast<uv_handle_t *>(socket_), LibuvCloseCb);
        }
        socket_ = nullptr;
    }
}

void NetWrapper::CSocket::SetNoDelay() {
    if (!socket_) {
        throw BaseException(__func__, "socket_ == nullptr");
    }
    LibuvNoDelay(socket_);
}

int NetWrapper::CSocket::Bind(const CAddress & address) {
    if (!socket_) {
        throw BaseException(__func__, "socket_ == nullptr");
    }
    
    struct sockaddr_in addr;
    Address2sockaddr_in(addr, address);
    int err = uv_tcp_bind(socket_, reinterpret_cast<const sockaddr *>(&addr), 0);
    if (0 == err) {
        return 0;
    }

    Close();
    OutputMsg(Logger::Error, "绑定ipv4端口(%s:%u)失败.(该端口已经被使用?)", address.GetAddress(), address.GetPort());
    return err;
}

int NetWrapper::CSocket::Bind6(const CAddress & address) {
    if (!socket_) {
        throw BaseException(__func__, "socket_ == nullptr");
    }

    struct sockaddr_in6 addr;
    Address2sockaddr_in6(addr, address);
    int err = uv_tcp_bind(socket_, reinterpret_cast<const sockaddr *>(&addr), 0);
    if (0 == err) {
        return 0;
    }

    Close();
    OutputMsg(Logger::Error, "绑定ipv6端口(%s:%u)失败.(该端口已经被使用?)", address.GetAddress(), address.GetPort());
    return err;
}

bool NetWrapper::CSocket::GetLocalAddress(CAddress & address) const {
    if (!socket_) {
        return false;
    }

    struct sockaddr_storage addr;
    int addr_len = sizeof(addr);
    int err = uv_tcp_getsockname(socket_, reinterpret_cast<struct sockaddr *>(&addr), &addr_len);
    if (0 != err) {
        OutputMsg(Logger::Error, "uv_tcp_getsockname error[%s]", uv_strerror(err));
        return false;
    }

    if (AF_INET == addr.ss_family) {
        char host[128] = { 0 };
        struct sockaddr_in * check_addr = reinterpret_cast<struct sockaddr_in *>(&addr);
        uv_ip4_name(check_addr, host, sizeof(host));
        address.SetAddress(host);
        address.SetPort(ntohs(check_addr->sin_port));
    } else if (AF_INET6 == addr.ss_family) {
        char host[128] = { 0 };
        struct sockaddr_in6 * check_addr = reinterpret_cast<struct sockaddr_in6 *>(&addr);
        uv_ip6_name(check_addr, host, sizeof(host));
        address.SetAddress(host);
        address.SetPort(ntohs(check_addr->sin6_port));
    }
    return true;
}

bool NetWrapper::CSocket::GetRemoteAddress(CAddress & address) const {
    if (!socket_) {
        return false;
    }

    struct sockaddr_storage addr;
    int addr_len = sizeof(addr);
    int err = uv_tcp_getpeername(socket_, reinterpret_cast<struct sockaddr *>(&addr), &addr_len);
    if (0 != err) {
        OutputMsg(Logger::Error, "uv_tcp_getpeername error[%s]", uv_strerror(err));
        return false;
    }

    if (AF_INET == addr.ss_family) {
        char host[128] = { 0 };
        struct sockaddr_in * check_addr = reinterpret_cast<struct sockaddr_in *>(&addr);
        uv_ip4_name(check_addr, host, sizeof(host));
        address.SetAddress(host);
        address.SetPort(ntohs(check_addr->sin_port));
    }
    else if (AF_INET6 == addr.ss_family) {
        char host[128] = { 0 };
        struct sockaddr_in6 * check_addr = reinterpret_cast<struct sockaddr_in6 *>(&addr);
        uv_ip6_name(check_addr, host, sizeof(host));
        address.SetAddress(host);
        address.SetPort(ntohs(check_addr->sin6_port));
    }
    return true;
}

void NetWrapper::CSocket::Address2sockaddr_in(sockaddr_in & addr, const CAddress & address) {
    int err = uv_ip4_addr(address.GetAddress(), address.GetPort(), &addr);
    if (0 != err) {
        OutputMsg(Logger::Error, "-- 域名出错:%s:%u --", address.GetAddress(), address.GetPort());
        OutputMsg(Logger::Info, "OnError: %d : %s .", err, uv_strerror(err));
        throw BaseException(__func__, "err != 0");
    }
}

void NetWrapper::CSocket::Address2sockaddr_in6(sockaddr_in6 & addr, const CAddress & address) {
    int err = uv_ip6_addr(address.GetAddress(), address.GetPort(), &addr);
    if (0 != err) {
        OutputMsg(Logger::Error, "-- 域名出错:%s:%u --", address.GetAddress(), address.GetPort());
        OutputMsg(Logger::Info, "OnError: %d : %s .", err, uv_strerror(err));
        throw BaseException(__func__, "err != 0");
    }
}

uv_tcp_t * NetWrapper::CSocket::CreateLibuvTcp(uv_loop_t * loop) {
    uv_tcp_t * tcp = new uv_tcp_t();
    int err = uv_tcp_init(loop, tcp);
    if (0 != err) {
        std::stringstream ss;
        ss << "uv_tcp_init error[" << uv_strerror(err) << "]";
        throw BaseException(__func__, ss.str());
    }
    return tcp;
}

void NetWrapper::CSocket::LibuvNoDelay(uv_tcp_t * tcp) {
    int err = uv_tcp_nodelay(tcp, 1);
    if (0 != err) {
        std::stringstream ss;
        ss << "uv_tcp_nodelay error[" << uv_strerror(err) << "]";
        throw BaseException(__func__, ss.str());
    }
}

void NetWrapper::CSocket::LibuvShutdownCb(uv_shutdown_t * req, int status) {
    if (!uv_is_closing(reinterpret_cast<uv_handle_t *>(req->handle))) {
        uv_close(reinterpret_cast<uv_handle_t *>(req->handle), LibuvCloseCb);
    }
    delete req;
}

void NetWrapper::CSocket::LibuvCloseCb(uv_handle_t * handle) {
    delete handle;
}