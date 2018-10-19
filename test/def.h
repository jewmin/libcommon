#ifndef __UNIT_TEST_DEF_H__
#define __UNIT_TEST_DEF_H__

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

#endif