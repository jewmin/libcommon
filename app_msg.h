#ifndef __LIB_COMMON_APP_MSG_H__
#define __LIB_COMMON_APP_MSG_H__

#include <stdint.h>

typedef struct {
    uint32_t msg_id; //消息号
    uint64_t param1; //消息参数1
    uint64_t param2; //消息参数2
    uint64_t param3; //消息参数3
    uint64_t param4; //消息参数4
    uint64_t param5; //消息参数5
} AppMessage;

#endif