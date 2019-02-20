#include "net_wrapper/connection.h"

NetWrapper::CConnection::CConnection(CConnectionMgr * mgr, uint32_t max_out_buffer_size, uint32_t max_in_buffer_size)
    : connection_mgr_(mgr), connection_id_(0), register_mgr_(false), raw_send_recv_(false), need_to_shutdown_(false) {
    if (!connection_mgr_) {
        throw BaseException(__func__, "connection_mgr_ == nullptr");
    }

    event_pipe_ = new CEventPipeImpl(this, max_out_buffer_size, max_in_buffer_size);
    if (!event_pipe_) {
        throw BaseException(__func__, "event_pipe_ == nullptr");
    }
}

NetWrapper::CConnection::~CConnection() {
    if (event_pipe_) {
        delete event_pipe_;
        event_pipe_ = nullptr;
    }
}

void NetWrapper::CConnection::OnConnected() {
    if (!register_mgr_) {
        connection_mgr_->
    }
    if (!m_bRegister2Mgr) {
        m_pConnectionMgr->RegisterConn(this);
    }
    m_pConnectionMgr->CallLuaAndRegTick(this);
    m_nPingTime = time(NULL);
    if (m_pConnectionMgr->m_nHandlerCount > 0) {
        int ret;
        int i = 0;
        do
        {
            CEventHandler* pHandler = (*m_pConnectionMgr->m_pHandlers)[i];
            ret = pHandler->OnConnected(this);
            if (ret == 1) {
                break;
            }
            else if (ret == 2) {
                NeedToShutdown(0);
                return;
            }
            if (ret != 0) {
                throw ArkCommon::CArkException("Assertion failure of expresion 'false'", __FILE__, __DATE__, __TIME__, __LINE__, __FUNCTION__, true, true);
            }
            ++i;
        } while (m_pConnectionMgr->m_nHandlerCount <= i);
    }
}

void NetWrapper::CConnection::OnConnectFailed(EPipeConnFailedReason reason) {

}

void NetWrapper::CConnection::OnDisconnect(EPipeDisconnectReason reason, bool remote) {

}

void NetWrapper::CConnection::OnDisconnected(EPipeDisconnectReason reason, bool remote) {

}

void NetWrapper::CConnection::OnNewDataReceived() {

}

void NetWrapper::CConnection::OnSomeDataSent() {

}

void NetWrapper::CConnection::OnError(int error) {

}

void NetWrapper::CConnection::BeforeWrite(const uint8_t * data, size_t len) {

}

void NetWrapper::CConnection::AfterWrite(size_t len) {

}

size_t NetWrapper::CConnection::SafeWrite(const uint8_t * data, size_t len) {

}

int NetWrapper::CConnection::Write(const uint8_t * data, size_t len) {

}

int NetWrapper::CConnection::Send(const uint8_t * data, size_t len) {

}

void NetWrapper::CConnection::NeedToShutdown(uint32_t connection_id) {

}

void NetWrapper::CConnection::NeedToShutdown(uint32_t connection_id, const char * reason) {

}

void NetWrapper::CConnection::Shutdown() {

}

void NetWrapper::CConnection::ShutdownNow() {

}

void NetWrapper::CConnection::OnPreAppShutDown() {

}

std::string NetWrapper::CConnection::GetInfo() {

}

uint8_t * NetWrapper::CConnection::ReverseGetOutputData(size_t len) {

}

uint8_t * NetWrapper::CConnection::GetRecvData() const {

}

size_t NetWrapper::CConnection::GetRecvDataSize() const {

}

void NetWrapper::CConnection::PopRecvData(size_t len) {

}

void NetWrapper::CConnection::SetMaxOutBufferSize(uint32_t size) {

}

void NetWrapper::CConnection::SetMaxInBufferSize(uint32_t size) {

}

size_t NetWrapper::CConnection::GetInBufferSize() {

}

size_t NetWrapper::CConnection::GetOutBufferSize() {

}

size_t NetWrapper::CConnection::GetOutBufferUsedSize() {

}

void NetWrapper::CConnection::DumpOutBuf() {

}

void NetWrapper::CConnection::DumpInBuf() {

}