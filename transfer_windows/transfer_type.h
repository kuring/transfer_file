#ifndef _TRANSFER_TYPE_H_
#define _TRANSFER_TYPE_H_


//#ifdef WIN32
// #define uint32_t unsigned int
// #define uint16_t unsigned short
// #define int16_t short
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef short int16_t;
typedef int int32_t;
//#endif

#define SRCIP 0x0100a8c0	// 192.168.0.2
#define DSTIP 0x0200a8c0	// 192.168.0.1

#define EPT_IP		 0x0800			/* type: IP	*/

#define UDP_PORT 0x401f				// UDP����˿ں�8000

typedef struct Ethernet
{
    unsigned char   eh_dst[6];      /* destination ethernet addrress */
    unsigned char   eh_src[6];      /* source ethernet addresss */
    unsigned short  eh_type;        /* ethernet pachet type */
}Ethernet;

typedef struct IpHeader              //����IP�ײ�
{
    unsigned char   h_verlen;       //4λ�ײ�����,4λIP�汾��
    unsigned char   tos;            //8λ��������TOS
    unsigned short  total_len;      //16λ�ܳ��ȣ��ֽڣ�
    unsigned short  ident;          //16λ��ʶ
    unsigned short  frag_and_flags; //3λ��־λ��13λƬƫ��
    unsigned char   ttl;            //8λ����ʱ�� TTL
    unsigned char   proto;          //8λЭ�� (TCP, UDP ������)
    unsigned short  checksum;       //16λIP�ײ�У���
    unsigned int    src_ip;         //32λԴIP��ַ
    unsigned int    dest_ip;        //32λĿ��IP��ַ
}IpHeader;

typedef struct UdpHeader
{
    unsigned short sport;
    unsigned short dport;
    unsigned short udplen;
    unsigned short crc16;
}UdpHeader;

// udp����ͷ
typedef struct UdpCheckHeader
{
	unsigned int    sourceIP;	//32λԴIP��ַ
	unsigned int    destIP;		//32λĿ��IP��ַ
	unsigned short  pro;		//0x1100
	unsigned short  udplen1;
}UdpCheckHeader;

enum PacketType
{
	SEND_FILE = 0,	// �����ļ������Ͷ˷��͸����ն�
	FILE_ACK,		// �ļ�����ȷ�ϣ����ն˷��͸����Ͷ�
	FILE_WRONG,		// �ļ�����ʧ�ܣ����ն˷��͸����Ͷ�
	SECOND_ACK,		// �ļ����͵ĵڶ���ȷ�ϣ��ɷ��Ͷ˷��͸��ͻ���
};

// �����ļ��İ�ͷ
typedef struct PacketHeader
{
	uint32_t session_id;	// �Ựid
	int32_t  block_offset;	// �����ݿ����ļ��е�ƫ�ƣ��ļ������ݿ�ò���ֵΪ-1����1��ʼ����
	uint16_t check_sum;		// ����У���
	uint16_t type;			// ��������
	uint16_t data_length;	// ���ݲ��ֳ���
	uint16_t send_count;	// �ڼ��η���
}PacketHeader;

#endif

