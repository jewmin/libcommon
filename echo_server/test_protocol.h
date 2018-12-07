#ifndef __TEST_PROTOCOL_H__
#define __TEST_PROTOCOL_H__

#include <stdint.h>

#pragma pack(1)

/* 服务器类型定义 */
typedef enum {
    InvalidServer = 0,  // 未定义服务器
    GateServer = 1,     // 网关服务器
    LogServer = 2,      // 日志服务器
    SessionServer = 3,  // 会话服务器
    DBServer = 4,       // 数据服务器
    GameServer = 5      // 游戏服务器
} server_type_t;

/*
* 服务器注册结构，当服务器连接到另一个服务器
* 需要发送注册服务器的数据结构以便注册服务器
* 注册服务器成功后才可正常通信
*/
typedef struct {
    static const int GT_ID = 0x12AA66CA;
    int game_type;          // 游戏ID，固定为0x12AA66CA
    int server_type;        // 服务器类型，值为SERVERTYPE枚举类型中的枚举值
    int server_index;       // 服务器唯一编号（服务器编号仅对游戏服务器有意义）
    int recv1;              // 保留字
    char server_name[64];   // 服务器名称，需包含0终止符
} server_regdata_t;

typedef struct tagPackHeader
{
    uint8_t pack_begin_flag;    // 0xBF

    uint16_t data_len;          // [PACK_HEADER] [DATA] /* data_len == sizeof(DATA) */
    uint16_t crc_data;          // [pack_begin_flag | pack_end_flag] | [data_len]

    uint8_t pack_end_flag;      // 0xEF
} PACK_HEADER;

#define PACK_BEGIN_FLAG 0xBF
#define PACK_END_FLAG   0xEF

#define _private_make_crc_data(x, y, z) ((x << 8 | y) | z)
#define MAKE_CRC_DATE(x, y, z) _private_make_crc_data(x, y, z)

#define PACK_HEADER_LEN sizeof(PACK_HEADER)

#pragma pack()

#endif