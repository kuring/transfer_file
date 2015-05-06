#include "stdafx.h"

#include <string.h>
#include <stdio.h>

#include <winsock2.h>


#include "transfer_type.h"
#include "file_sender.h"
#include "public_var.h"
#include "public_func.h"
#include "session.h"

DWORD WINAPI send_file_thread(void *p)
{
	FileSender *send_file = (FileSender*)p;
	while (1)
	{
		Sleep(1000);
		send_file->send_file_from_queue();
	}
	return 1;
}


FileSender::FileSender()
{
	queue_mutex = CreateMutex(NULL, FALSE, NULL);
	DWORD tid;
	if (send_file_t = CreateThread(NULL, 0, send_file_thread, this, 0, &tid))
	{
		CloseHandle(send_file_t);
	}
	file_sender_map = new FileSenderMap;
}

FileSender::~FileSender()
{
	CloseHandle(queue_mutex);
	if (file_sender_map != NULL)
	{
		delete file_sender_map;
	}
	file_sender_map = NULL;
}

bool FileSender::add_send_file(const char *filename)
{
	// ����ļ��Ƿ����
	if (access(filename, 0) != 0)
	{
		return false;
	}

	// FileSenderUnit���ͷ���FileSenderMap����
	FileSenderUnit *unit = new FileSenderUnit;
	unit->filename = filename;
	unit->session_id = session.alloc_sessionid();

	add_send_file(unit);
	return true;
}

bool FileSender::add_send_file(FileSenderUnit *unit)
{
	WaitForSingleObject(queue_mutex, INFINITE);
	// ����ļ��Ƿ��Ѿ��ڷ��Ͷ�����
	std::deque<FileSenderUnit *>::iterator itor = file_sender_queue.begin();
	for (; itor != file_sender_queue.end(); itor++)
	{
		if ((*itor) == unit)
		{
			break;
		}
	}
	if (itor == file_sender_queue.end())
	{
		// ��ǰ�ļ������ļ����Ͷ�����
		unit->send_count++;
		this->file_sender_queue.push_back(unit);
	}
	ReleaseMutex(queue_mutex);
	return true;
}

void FileSender::send_file_from_queue()
{
	WaitForSingleObject(queue_mutex, INFINITE);
	if (this->file_sender_queue.size() > 0)
	{
		FileSenderUnit *unit = this->file_sender_queue.front();
		this->file_sender_queue.pop_front();
		ReleaseMutex(queue_mutex);
		send(unit);
	}
	else
	{
		ReleaseMutex(queue_mutex);
	}
}

int FileSender::send(FileSenderUnit *unit)
{
	if (unit->filename.size() <= 0)
	{
		return -1;
	}

	FILE *file = fopen(unit->filename.c_str(), "rb");
	if (file == NULL)
	{
		return -1;
	}

	WriteLog("��ʼ�������Ϸ����ļ�%s,��ǰ�ļ����ʹ���%d", unit->filename.c_str(), unit->send_count);

	// ��ȡҪ���͵��ļ�����
	int file_length = get_file_length(file);
	if (file_length == -1)
	{
		return -1;
	}

	int block_num = file_length / BLOCK_SIZE;
	if (file_length % BLOCK_SIZE > 0)
	{
		block_num++;
	}

	unsigned char packet_buffer[2000];
	memset(packet_buffer, 0, 2000);

	// �����ļ��ܳ��ȡ��ļ��ܿ������ļ���
	int packet_header_length = sizeof(PacketHeader);
	PacketHeader *packet_header = (PacketHeader*)packet_buffer;
	packet_header->type = SEND_FILE;
	packet_header->session_id = unit->session_id;
	packet_header->data_length = unit->filename.size() + 8;
	packet_header->block_offset = -1;
	packet_header->send_count = unit->send_count;
	uint32_t tmp = htonl(file_length);
	memcpy(packet_buffer + packet_header_length, &tmp, 4); 	// �����ļ�����
	tmp = htonl(block_num);
	memcpy(packet_buffer + packet_header_length + 4, &tmp, 4);  // ����Ҫ���͵��ļ��ܿ���
	memcpy(packet_buffer + packet_header_length + 8, unit->filename.c_str(), packet_header->data_length);	// �����ļ���
	packet_header->check_sum = check_sum(packet_buffer + packet_header_length, packet_header->data_length);
	g_netcard_writer->add_send_queue(packet_buffer, packet_header_length + packet_header->data_length);

	WriteLog("send file, file_length=%d,block_num=%d",
			ntohl(*(int *) (packet_buffer + sizeof(PacketHeader))),
			ntohl(*(int *) (packet_buffer + sizeof(PacketHeader) + 4)));

	// �����ļ�����
	for (int i=1; i<=block_num; i++)
	{
		int off = fread(packet_buffer + packet_header_length, 1, BLOCK_SIZE, file);
		if (off <= 0)
		{
			break;
		}
		packet_header->block_offset = i;
		packet_header->data_length = off;
		packet_header->check_sum = check_sum(packet_buffer + packet_header_length, off);
		g_netcard_writer->add_send_queue(packet_buffer, packet_header_length + packet_header->data_length);
	}

	fclose(file);

	// ���ļ����浽Ҫ�ѷ��͵��ļ�������
	file_sender_map->add_map(packet_header->session_id, unit);
	return 1;
}
