#include <string.h>

#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <netpacket/packet.h>
#include <sys/ioctl.h>
#include <linux/if.h>

#include "netcard_reader_raw.h"
#include "transfer_type.h"
#include "inifile.h"
#include "public_func.h"
#include "public_var.h"

void * netcard_reader_thread(void *)
{
	g_netcard_reader = new NetcardReaderRaw();
	g_netcard_reader->receive();
	return NULL;
}

NetcardReaderRaw::NetcardReaderRaw()
{
	find_device();
	open();
	this->file_saver = new FileSaver();
}

NetcardReaderRaw::~NetcardReaderRaw()
{
	if (this->file_saver != NULL)
	{
		delete this->file_saver;
	}
	this->file_saver = NULL;
}

bool NetcardReaderRaw::find_device()
{
    int result = IniFile::read_profile_string("netcard", "receive", device_name, 20, "", "transfer_file.ini");
    if (result == 1)
    {
        return true;
    }
    return false;
}

bool NetcardReaderRaw::open()
{
	fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (fd == -1)
	{
		WriteLog("接收网卡打开数据链路层失败");
		return false;
	}

	// 设置网卡混杂模式
	struct ifreq ifr;
	memset(&ifr, 0, sizeof(struct ifreq));
	strcpy(ifr.ifr_name, device_name);
	if (ioctl(fd, SIOCGIFINDEX, &ifr) < 0)
	{
		WriteLog("未获取到设备对应的索引号");
		return false;
	}

	packet_mreq mreq;
	mreq.mr_type = PACKET_MR_PROMISC;
	mreq.mr_ifindex = ifr.ifr_ifindex;
	mreq.mr_alen = 0;
	mreq.mr_address[1] = '\0';

	if (setsockopt(fd, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mreq, sizeof(packet_mreq)) < 0)
	{
		WriteLog("setsockopt错误");
		return false;
	}

	// 绑定到指定设备
	struct sockaddr_ll addr;
	memset(&addr, 0, sizeof(sockaddr_ll));
	addr.sll_family = PF_PACKET;
	addr.sll_protocol = htons(ETH_P_ALL);
	addr.sll_ifindex = ifr.ifr_ifindex;
	if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
	{
		WriteLog("绑定到网卡%s失败", device_name);
		return false;
	}

	WriteLog("读取网卡打开网卡%s成功", device_name);
	return true;
}

bool NetcardReaderRaw::receive()
{
	while (1)
	{
		unsigned char buffer[1500];
		int size = recvfrom(fd, buffer, 1500, 0, NULL, NULL);
		if (size < 0)
		{
			WriteLog("接收数据包错误");
		}
		deal_raw_packet(buffer, size);
	}
	return true;
}

bool NetcardReaderRaw::deal_raw_packet(const unsigned char *packet, int size)
{
	Ethernet *ethernet = (Ethernet *)packet;
	if (ethernet->eh_type != 0x0008)
	{
		return false;
	}

	IpHeader *ip_header = (IpHeader *)(packet + 14);
	if(ip_header->proto != 0x11)
	{
		return false;
	}

	if (ip_header->dest_ip != DSTIP || ip_header->src_ip != SRCIP)
	{
		return false;
	}

	UdpHeader *udp_header = (UdpHeader *)(packet + 14 + sizeof(IpHeader));
	if (udp_header->dport != udp_header->sport)
	{
		return false;
	}

	// 检查udp数据包的长度
	unsigned int data_length = ntohs(ip_header->total_len) - sizeof(IpHeader) - sizeof(UdpHeader);
	if (data_length < sizeof(PacketHeader))
	{
		return false;
	}

	// 检测udp校验和
//	unsigned short checksum = check_sum(udp_header, udp_header->udplen);
//	if (checksum != udp_header->crc16)
//	{
//		return ;
//	}

	// 开始处理包
	const unsigned char *data = packet + 14 + sizeof(IpHeader) + sizeof(UdpHeader);
	PacketHeader *packet_header = (PacketHeader *)data;
	if (packet_header->type == FILE_ACK)
	{
		// 接收到文件发送成功包
		g_file_sender->file_sender_map->deal_ack(packet_header->session_id);
		WriteLog("发送端：发送端接收到客户端的session为%d的ack包", packet_header->session_id);
	}
	else if (packet_header->type == SEND_FILE)
	{
		g_netcard_reader->file_saver->save(data);
	}
	else if (packet_header->type == FILE_WRONG)
	{
		// 接收到文件发送失败包
		//WriteLog("接收到文件发送失败包，sessionid=%d,send_count=%d", packet_header->session_id, packet_header->send_count);
		g_netcard_writer->clear_send_queue(packet_header->session_id, packet_header->send_count);
		g_file_sender->file_sender_map->deal_send_fail(packet_header->session_id, packet_header->send_count);
	}
	else
	{
		WriteLog("接收到未识别的包");
	}
	return true;
}
