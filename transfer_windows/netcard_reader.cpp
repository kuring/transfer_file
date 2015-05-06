#include "stdafx.h"

#include <string.h>


#include "netcard_reader.h"
#include "public_func.h"
#include "transfer_type.h"
#include "public_var.h"

NetcardReader::NetcardReader()
{
	if (!open())
	{
		WriteLog("打开接收网卡错误");
		_exit(-1);
	}
	this->file_saver = new FileSaver();
}

NetcardReader::~NetcardReader()
{
	if (this->pcap_read != NULL)
	{
		pcap_close(this->pcap_read);
	}
	this->pcap_read = NULL;

	if (this->file_saver != NULL)
	{
		delete this->file_saver;
	}
	this->file_saver = NULL;
}

bool NetcardReader::find_device()
{
    GetPrivateProfileString("netcard", "receive", "", device_name, 20, "E:\\transfer_file\\para\\transfer_file.ini");
    if (strlen(device_name) > 0)
    {
        return true;
    }
    return false;
}

bool NetcardReader::open()
{
    char error_buf[PCAP_ERRBUF_SIZE];
    if (find_device() == false || strlen(device_name) == 0)
    {
        return false;
    }
    pcap_if_t *alldevs;
	if (pcap_findalldevs(&alldevs, error_buf) == -1)
	{
		return false;
	}
	pcap_if_t *device;
	for (int i=0; i<strlen(device_name); i++)
	{
		if (!isdigit(device_name[i]))
		{
			WriteLog("请将网卡的名字更改为数字格式");
			return false;
		}
	}
	int number = atoi(device_name);
	int j = 0;
	for (device = alldevs; device != NULL; device = device->next)
	{		
		if (++j == number)
		{
			break;
		}	
	}
	this->pcap_read = pcap_open_live(device->name, 1500, 1, 20, error_buf);
	pcap_freealldevs(alldevs);
    if (this->pcap_read == NULL)
    {
    	WriteLog("打开接收网卡%s失败，程序退出", device_name);
        return false;
    }
    return true;
}

bool NetcardReader::receive()
{
	char *filter = "src host 192.168.0.1 and udp port 8000";
	struct bpf_program fcode;

	if (pcap_compile(pcap_read, &fcode, filter, 1, 1) < 0)
	{
		WriteLog("%s", pcap_geterr(pcap_read));
		pcap_close(pcap_read);
	}
	if (pcap_setfilter(pcap_read, &fcode) < 0)
	{
		WriteLog("%s", pcap_geterr(pcap_read));
		pcap_close (pcap_read);
	}

	WriteLog("读网卡数据类打开网卡%s成功", device_name);
	pcap_loop(pcap_read, -1, pcap_callback, NULL);
	return true;
}

void pcap_callback(u_char *useless, const struct pcap_pkthdr* pkthdr, const u_char *packet)
{
	Ethernet *ethernet = (Ethernet *)packet;
	if (ethernet->eh_type != 0x0008)
	{
		return ;
	}

	IpHeader *ip_header = (IpHeader *)(packet + 14);
	if(ip_header->proto != 0x11)
	{
		return ;
	}

	UdpHeader *udp_header = (UdpHeader *)(packet + 14 + sizeof(IpHeader));
	if (udp_header->dport != udp_header->sport)
	{
		return ;
	}

	// 检查udp数据包的长度
	unsigned int data_length = ntohs(ip_header->total_len) - sizeof(IpHeader) - sizeof(UdpHeader);
	if (data_length < sizeof(PacketHeader))
	{
		return ;
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
		WriteLog("发送端接收到客户端的session为%d的ack包", packet_header->session_id);
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
}
