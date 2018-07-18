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
    virtual void OnRecvMsg() { recv_count++; }
};

#endif