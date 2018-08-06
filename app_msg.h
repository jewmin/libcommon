#ifndef __LIB_COMMON_APP_MSG_H__
#define __LIB_COMMON_APP_MSG_H__

#include <stdint.h>

typedef struct {
    uint32_t msg_id; //��Ϣ��
    uint64_t param1; //��Ϣ����1
    uint64_t param2; //��Ϣ����2
    uint64_t param3; //��Ϣ����3
    uint64_t param4; //��Ϣ����4
    uint64_t param5; //��Ϣ����5
} AppMessage;

#endif