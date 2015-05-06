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

#define UDP_PORT 0x401f				// UDP传输端口号8000

typedef struct Ethernet
{
    unsigned char   eh_dst[6];      /* destination ethernet addrress */
    unsigned char   eh_src[6];      /* source ethernet addresss */
    unsigned short  eh_type;        /* ethernet pachet type */
}Ethernet;

typedef struct IpHeader              //定义IP首部
{
    unsigned char   h_verlen;       //4位首部长度,4位IP版本号
    unsigned char   tos;            //8位服务类型TOS
    unsigned short  total_len;      //16位总长度（字节）
    unsigned short  ident;          //16位标识
    unsigned short  frag_and_flags; //3位标志位和13位片偏移
    unsigned char   ttl;            //8位生存时间 TTL
    unsigned char   proto;          //8位协议 (TCP, UDP 或其他)
    unsigned short  checksum;       //16位IP首部校验和
    unsigned int    src_ip;         //32位源IP地址
    unsigned int    dest_ip;        //32位目的IP地址
}IpHeader;

typedef struct UdpHeader
{
    unsigned short sport;
    unsigned short dport;
    unsigned short udplen;
    unsigned short crc16;
}UdpHeader;

// udp检验头
typedef struct UdpCheckHeader
{
	unsigned int    sourceIP;	//32位源IP地址
	unsigned int    destIP;		//32位目的IP地址
	unsigned short  pro;		//0x1100
	unsigned short  udplen1;
}UdpCheckHeader;

enum PacketType
{
	SEND_FILE = 0,	// 发送文件，发送端发送给接收端
	FILE_ACK,		// 文件接收确认，接收端发送给发送端
	FILE_WRONG,		// 文件传输失败，接收端发送给发送端
	SECOND_ACK,		// 文件发送的第二次确认，由发送端发送给客户端
};

// 传输文件的包头
typedef struct PacketHeader
{
	uint32_t session_id;	// 会话id
	int32_t  block_offset;	// 该数据块在文件中的偏移，文件名数据块该部分值为-1，从1开始计算
	uint16_t check_sum;		// 包的校验和
	uint16_t type;			// 传输类型
	uint16_t data_length;	// 数据部分长度
	uint16_t send_count;	// 第几次发送
}PacketHeader;

#endif

