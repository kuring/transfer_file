#include "stdafx.h"

#include "string.h"

#include "netcard_writer.h"
#include "public_func.h"
#include "transfer_type.h"

DWORD WINAPI write_netcard_thread(void *p)
{
	NetcardWriter *write_netcard = (NetcardWriter *)p;
	while (1)
	{
		write_netcard->send_to_card();
		Sleep(1000);
	}
	return 1;
}


NetcardWriter::NetcardWriter()
{
    queue_mutex = CreateMutex(NULL, FALSE, NULL);
    open();
}

NetcardWriter::~NetcardWriter()
{
    if (this->pcap_write != NULL)
    {
        pcap_close(this->pcap_write);
    }
    this->pcap_write = NULL;
    CloseHandle(queue_mutex);
}

bool NetcardWriter::find_device()
{
    GetPrivateProfileString("netcard", "send", "", device_name, 20, "E:\\transfer_file\\para\\transfer_file.ini");
    if (strlen(device_name) > 0)
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
			WriteLog("�뽫���������ָ���Ϊ���ָ�ʽ");
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
	this->pcap_write = pcap_open_live(device->name, 1500, 1, 0, error_buf);
	pcap_freealldevs(alldevs);
    if (this->pcap_write == NULL)
    {
        return false;
    }
    WriteLog("д���������������%s�ɹ�", device_name);

    // ��ȡ���ݿ鷢�Ͷ��в������ݿ鷢�͵������߳�
    	DWORD tid;
	if (write_netcard_t = CreateThread(NULL, 0, write_netcard_thread, this, 0, &tid))
	{
		WriteLog("���Ͷˣ���ȡ���ݿ鷢�Ͷ����߳������ɹ�");
		CloseHandle(write_netcard_t);
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

	// �޸�UDPУ��ͣ�����udpα�ײ���udpͷ����udp�����ݲ���
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

	WaitForSingleObject(queue_mutex, INFINITE);
	this->send_queue.push(unit);
	ReleaseMutex(queue_mutex);
}

int NetcardWriter::send_to_card()
{
	while (1)
	{
		// ����һ�������е����ݷ���ڶ���������
		// ÿ�����ȡ100��
		WaitForSingleObject(queue_mutex, INFINITE);
		if (this->send_queue.empty())
		{
			ReleaseMutex(queue_mutex);
			break;
		}

		int count = 0;
		while (!this->send_queue.empty() && count++ < 50)
		{
			SendQueueUnit *unit = send_queue.front();
			send_buffer.push_back(unit);
			send_queue.pop();
		}
		ReleaseMutex(queue_mutex);

		// �������Ϸ��͵ڶ����������е�����
		int size = send_buffer.size();
		if (size > 0)
		{
			for (int i=0; i<size; i++)
			{
				SendQueueUnit *unit = send_buffer[i];				
				PacketHeader *header = (PacketHeader *)(unit->buffer + sizeof(Ethernet) + sizeof(IpHeader) + sizeof(UdpHeader));
				// �����ļ��ĵ�һ����֮ǰ˯��һ�ᣬ�ȴ����ն˽���һ���ļ�д�����
				if (header->block_offset == -1)
				{
					Sleep(1000);
				}
				pcap_sendpacket(this->pcap_write, unit->buffer, unit->length);
				// ����Ƿ����ļ��ĵ�һ������˯��һС��ʱ����ٷ��ͣ��Ա�ȴ����ն������ڴ�
				if (header->block_offset == -1)
				{
					WriteLog("��⵽���͵����ļ���һ����");
					Sleep(500);
				}
				delete unit;
			}
			send_buffer.clear();
			//WriteLog("���Ͷˣ��������Ϸ���%d�����ݰ�, %d", size, send_queue.size());
		}
		Sleep(1);
	}
	return 1;
}

void NetcardWriter::clear_send_queue(int session_id, uint16_t send_count)
{
	WaitForSingleObject(queue_mutex, INFINITE);
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
	ReleaseMutex(queue_mutex);
}

bool NetcardWriter::check_send_finish(int session_id)
{
	bool result = true;
	WaitForSingleObject(queue_mutex, INFINITE);
	if (!send_queue.empty())
	{
		SendQueueUnit *unit = send_queue.front();
		if (unit->session_id == session_id)
		{
			result = false;
		}
	}
	ReleaseMutex(queue_mutex);
	return result;
}
