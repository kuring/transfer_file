#include <arpa/inet.h>
#include <unistd.h>

#include "string.h"

#include "netcard_writer.h"
#include "inifile.h"
#include "public_func.h"
#include "transfer_type.h"

void * write_netcard_thread(void *p)
{
	NetcardWriter *write_netcard = (NetcardWriter *)p;
	while (1)
	{
		write_netcard->send_to_card();
		sleep(1);
	}
	return (void *)1;
}

NetcardWriter::NetcardWriter()
{
    pthread_mutex_init(&queue_mutex, NULL);
    open();
}

NetcardWriter::~NetcardWriter()
{
    if (this->pcap_write != NULL)
    {
        pcap_close(this->pcap_write);
    }
    this->pcap_write = NULL;
    pthread_mutex_destroy(&queue_mutex);
}

bool NetcardWriter::find_device()
{
    int result = IniFile::read_profile_string("netcard", "send", device_name, 20, "", "transfer_file.ini");
    if (result == 1)
    {
        return true;
    }
    return false;
}

bool NetcardWriter::open()
{
    char error_buf[PCAP_ERRBUF_SIZE];
    if (find_device() == false || strlen(device_name) == 0)
    {
        return false;
    }
    this->pcap_write = pcap_open_live(device_name, 1500, 1, 0, error_buf);
    if (this->pcap_write == NULL)
    {
        return false;
    }
    WriteLog("写网卡数据类打开网卡%s成功", device_name);

    // 读取数据块发送队列并将数据块发送到网卡线程
    int result = pthread_create(&write_netcard_t, NULL, write_netcard_thread, this);
    if (result == 0)
    {
    	WriteLog("发送端：读取数据块发送队列线程启动成功");
    	pthread_detach(write_netcard_t);
    }

    return true;
}

void NetcardWriter::add_send_queue(unsigned char *buffer, int length)
{
	unsigned short tempipid=0;

	// ethernet
	Ethernet ethhdr;
	ethhdr.eh_type = htons(EPT_IP);
	memcpy(ethhdr.eh_src, "\x00\x5\x4\x2\x1\x1", 6);
	memcpy(ethhdr.eh_dst, "\x00\x2\x3\x4\x5\x6", 6);

	// ip
	IpHeader ip_header;
	ip_header.h_verlen = (4 << 4) | (sizeof(IpHeader) / sizeof(unsigned int));
	ip_header.tos = 0;
	ip_header.total_len = htons(sizeof(IpHeader) + sizeof(UdpHeader) + length);
	ip_header.frag_and_flags = 0;
	ip_header.ttl = 128;
	ip_header.proto = IPPROTO_UDP;
	ip_header.ident = tempipid;
	ip_header.src_ip = SRCIP;
	ip_header.dest_ip = DSTIP;
	ip_header.checksum = 0;
	ip_header.checksum = check_sum(&ip_header, sizeof(IpHeader));

	// udp
	UdpHeader udp_header;
	udp_header.sport = UDP_PORT; //0x401f;
	udp_header.dport = UDP_PORT; //0x401f;
	udp_header.crc16 = 0;
	udp_header.udplen = htons(sizeof(UdpHeader) + length);

	SendQueueUnit *unit = new SendQueueUnit();
	unit->length = sizeof(Ethernet) + sizeof(IpHeader) + sizeof(UdpHeader) + length;
	unit->buffer = new unsigned char[unit->length];
	unit->session_id = ((PacketHeader *)buffer)->session_id;
	memset(unit->buffer, 0, unit->length);
	memcpy(unit->buffer, &ethhdr, sizeof(Ethernet));
	memcpy(unit->buffer + sizeof(Ethernet), &ip_header, sizeof(IpHeader));
	memcpy(unit->buffer + sizeof(Ethernet) + sizeof(IpHeader), &udp_header, sizeof(UdpHeader));
	memcpy(unit->buffer + sizeof(Ethernet) + sizeof(IpHeader) + sizeof(UdpHeader), buffer, length);

	// 修改UDP校验和，包括udp伪首部、udp头部和udp的数据部分
	unsigned char udp_check_buffer[2000];
	memset(udp_check_buffer, 0, 2000);
	UdpCheckHeader *check_header = (UdpCheckHeader *)udp_check_buffer;
	check_header->sourceIP = ip_header.src_ip;
	check_header->destIP = ip_header.dest_ip;
	check_header->pro = htons(0x0011);
	check_header->udplen1 = udp_header.udplen;
	UdpHeader* udp = (UdpHeader*)(unit->buffer + sizeof(Ethernet) + sizeof(IpHeader));
	memcpy(udp_check_buffer + sizeof(UdpCheckHeader), udp, sizeof(UdpHeader) + length);
	unsigned short sum = check_sum(udp_check_buffer, sizeof(UdpCheckHeader) + sizeof(UdpHeader) + length);
	udp->crc16 = sum;

	pthread_mutex_lock(&queue_mutex);
	this->send_queue.push(unit);
	pthread_mutex_unlock(&queue_mutex);
}

int NetcardWriter::send_to_card()
{
	while (1)
	{
		// 将第一个队列中的数据放入第二个缓冲区
		// 每次最多取100个
		pthread_mutex_lock(&queue_mutex);
		if (this->send_queue.empty())
		{
			pthread_mutex_unlock(&queue_mutex);
			break;
		}

		int count = 0;
		while (!this->send_queue.empty() && count++ < 50)
		{
			SendQueueUnit *unit = send_queue.front();
			send_buffer.push_back(unit);
			send_queue.pop();
		}
		pthread_mutex_unlock(&queue_mutex);

		// 向网卡上发送第二个缓冲区中的数据
		int size = send_buffer.size();
		if (size > 0)
		{
			for (int i=0; i<size; i++)
			{
				SendQueueUnit *unit = send_buffer[i];
				PacketHeader *header = (PacketHeader *)(unit->buffer + sizeof(Ethernet) + sizeof(IpHeader) + sizeof(UdpHeader));
				// 发送文件的第一个包之前睡眠一会，等待接收端将上一个文件写入磁盘
				if (header->block_offset == -1)
				{
					sleep(1);
				}
				pcap_sendpacket(this->pcap_write, unit->buffer, unit->length);
				// 如果是发送文件的第一个包则睡眠一小段时间后再发送，以便等待接收端申请内存
				if (header->block_offset == -1)
				{
					WriteLog("检测到发送的是文件第一个包");
					sleep(1);
				}
				delete unit;
			}
			send_buffer.clear();
			//WriteLog("发送端：向网卡上发送%d个数据包, %d", size, send_queue.size());
		}
		usleep(800);
	}
	return 1;
}

void NetcardWriter::clear_send_queue(int session_id, uint16_t send_count)
{
	pthread_mutex_lock(&queue_mutex);
	while (!send_queue.empty())
	{
		if (send_queue.front()->session_id == session_id)
		{
			PacketHeader *header =
					(PacketHeader *)(send_queue.front()->buffer + sizeof(Ethernet) + sizeof(IpHeader) + sizeof(UdpHeader));
			if (header->send_count == send_count)
			{
				send_queue.pop();
			}
			else
			{
				break;
			}
		}
		else
		{
			break;
		}
	}
	pthread_mutex_unlock(&queue_mutex);
}

bool NetcardWriter::check_send_finish(int session_id)
{
	bool result = true;
	pthread_mutex_lock(&queue_mutex);
	if (!send_queue.empty())
	{
		SendQueueUnit *unit = send_queue.front();
		if (unit->session_id == session_id)
		{
			result = false;
		}
	}
	pthread_mutex_unlock(&queue_mutex);
	return result;
}
