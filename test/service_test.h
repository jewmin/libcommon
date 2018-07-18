#ifndef __UNIT_TEST_SERVICE_TEST_H__
#define __UNIT_TEST_SERVICE_TEST_H__

#include "gmock/gmock.h"
#include "service.h"

class MockService : public BaseService
{
public:
    int recv_count;

    MockService() { recv_count = 0; }
    virtual ~MockService() {}
	virtual void OnRecvMsg(uint32_t msg_id, uint64_t param1, uint64_t param2, uint64_t param3, uint64_t param4, uint64_t param5) { recv_count++; }
};

#endif