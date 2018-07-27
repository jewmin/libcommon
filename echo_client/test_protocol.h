#ifndef __TEST_PROTOCOL_H__
#define __TEST_PROTOCOL_H__

#include "stdint.h"

#pragma pack(1)

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
#define MAKE_CRC_DATE (x, y, z) _private_make_crc_data(x, y, z)

#define PACK_HEADER_LEN sizeof(PACK_HEADER)

#pragma pack()

#endif