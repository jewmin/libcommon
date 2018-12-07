#ifndef __TEST_PROTOCOL_H__
#define __TEST_PROTOCOL_H__

#include <stdint.h>

#pragma pack(1)

/* ���������Ͷ��� */
typedef enum {
    InvalidServer = 0,  // δ���������
    GateServer = 1,     // ���ط�����
    LogServer = 2,      // ��־������
    SessionServer = 3,  // �Ự������
    DBServer = 4,       // ���ݷ�����
    GameServer = 5      // ��Ϸ������
} server_type_t;

/*
* ������ע��ṹ�������������ӵ���һ��������
* ��Ҫ����ע������������ݽṹ�Ա�ע�������
* ע��������ɹ���ſ�����ͨ��
*/
typedef struct {
    static const int GT_ID = 0x12AA66CA;
    int game_type;          // ��ϷID���̶�Ϊ0x12AA66CA
    int server_type;        // ���������ͣ�ֵΪSERVERTYPEö�������е�ö��ֵ
    int server_index;       // ������Ψһ��ţ���������Ž�����Ϸ�����������壩
    int recv1;              // ������
    char server_name[64];   // ���������ƣ������0��ֹ��
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