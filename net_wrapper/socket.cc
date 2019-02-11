#include "net_wrapper/socket.h"

NetWrapper::CSocket::CSocket()
    : tcp_(nullptr), is_shutdown_read_(true), is_shutdown_write_(true) {

}

NetWrapper::CSocket::~CSocket() {
    Close();
}

void NetWrapper::CSocket::Open(CEventReactor * event_reactor) {
    if (tcp_) {
        throw BaseException(__func__, "tcp_ != nullptr");
    }

    tcp_ = CreateLibuvTcp(event_reactor->GetLibuvLoop());
    is_shutdown_read_ = false;
    is_shutdown_write_ = false;
}

void NetWrapper::CSocket::Close() {
    if (tcp_) {
        if (!uv_is_closing(reinterpret_cast<uv_handle_t *>(tcp_))) {
            uv_close(reinterpret_cast<uv_handle_t *>(tcp_), LibuvCloseCb);
        }
        tcp_ = nullptr;
    }
}

void NetWrapper::CSocket::SetNoDelay() {
    if (!tcp_) {
        throw BaseException(__func__, "tcp_ == nullptr");
    }

    LibuvNoDelay(tcp_);
}

int NetWrapper::CSocket::Bind(const CAddress & address) {
    return BindImpl(address, ADDRESSFAMILY_IPV4);
}

int NetWrapper::CSocket::Bind6(const CAddress & address) {
    return BindImpl(address, ADDRESSFAMILY_IPV6);
}

int NetWrapper::CSocket::BindImpl(const CAddress & address, EAddressFamily family) {
    if (!tcp_) {
        throw BaseException(__func__, "tcp_ == nullptr");
    }

    union {
        struct sockaddr addr;
        struct sockaddr_in addr4;
        struct sockaddr_in6 addr6;
    } s;
    if (ADDRESSFAMILY_IPV4 == family) {
        Address2sockaddr_in(s.addr4, address);
    } else {
        Address2sockaddr_in6(s.addr6, address);
    }

    int err = uv_tcp_bind(tcp_, &s.addr, 0);
    if (0 == err) {
        return 0;
    }

    Close();
    OutputMsg(Logger::Error, "绑定端口(%s:%u)失败.(该端口已经被使用?)", address.GetAddress(), address.GetPort());
    return err;
}

bool NetWrapper::CSocket::GetLocalAddress(CAddress & address) const {
    return GetAddressImpl(address, ADDRESSTYPE_LOCAL);
}

bool NetWrapper::CSocket::GetRemoteAddress(CAddress & address) const {
    return GetAddressImpl(address, ADDRESSTYPE_REMOTE);
}

bool NetWrapper::CSocket::GetAddressImpl(CAddress & address, EAddressType type) const {
    if (!tcp_) {
        return false;
    }

    struct sockaddr_storage addr;
    int addr_len = sizeof(addr);
    int err;
    if (ADDRESSTYPE_LOCAL == type) {
        err = uv_tcp_getsockname(tcp_, reinterpret_cast<struct sockaddr *>(&addr), &addr_len);
    } else {
        err = uv_tcp_getpeername(tcp_, reinterpret_cast<struct sockaddr *>(&addr), &addr_len);
    }
    if (0 != err) {
        OutputMsg(Logger::Error, "%s error[%s]", ADDRESSTYPE_LOCAL == type ? "uv_tcp_getsockname" : "uv_tcp_getpeername", uv_strerror(err));
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