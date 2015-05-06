#include "stdafx.h"

#include <string.h>

#include <winsock2.h>

#include "file_saver.h"
#include "transfer_type.h"
#include "netcard_writer.h"
#include "public_var.h"
#include "public_func.h"

FileSaver::FileSaver()
{
}

FileSaver::~FileSaver()
{
	// ���file_map�е�����
	std::map<unsigned int, FileSaverUnit *>::iterator iter;
	for(iter = file_map.begin(); iter != file_map.end(); iter++)
	{
		delete iter->second;
		file_map.erase(iter);
	}
}

bool FileSaver::save(const unsigned char *buffer)
{
	PacketHeader *header = (PacketHeader *)buffer;
	if (file_map.find(header->session_id) == file_map.end())
	{
		if (header->block_offset == -1)	// first packet
		{
			int file_length = ntohl(*(int *)(buffer + sizeof(PacketHeader)));
			int block_num = ntohl(*(int *)(buffer + sizeof(PacketHeader) + 4));
			// ��ȡ�ļ������˴�������ڴ���SaveFileUnit��������������ͷ�
			int filename_length = header->data_length - 8;
			FileSaverUnit *unit = new FileSaverUnit();
			unit->create(file_length, block_num, buffer + sizeof(PacketHeader) + 8, filename_length);
			file_map[header->session_id] = unit;
			WriteLog("���ն˽��յ��ļ�����%d���ļ�����%d���ļ�", file_length, block_num);
		}
	}
	else
	{
		if (header->block_offset > 0)	// �ļ����ݿ�
		{
			std::map<unsigned int, FileSaverUnit *>::iterator itor = file_map.find(header->session_id);
			if (itor == file_map.end())
			{
				return false;
			}
			int result = itor->second->save(buffer);
			if (result == 2)
			{
				// ���ļ���Ϣ��map��ɾ��
				file_map.erase(itor);
				// �ļ����ճɹ�
				PacketHeader send_header;
				send_header.session_id = header->session_id;
				send_header.type = FILE_ACK;
				send_header.send_count = header->send_count;
				g_netcard_writer->add_send_queue((unsigned char *)&send_header, sizeof(PacketHeader));
				WriteLog("���ն����Ͷ˷����ļ����ճɹ�");
				return true;
			}
			else if (result == -1)
			{
				// �ļ����ݿ����ʧ�ܣ����ܽ��յ��ļ����ݿ��м䶪����
				PacketHeader send_header;
				send_header.session_id = header->session_id;
				send_header.send_count = header->send_count;
				send_header.block_offset = 0;
				send_header.type = FILE_WRONG;
 				g_netcard_writer->add_send_queue((unsigned char *)&send_header, sizeof(PacketHeader));
 				WriteLog("�����ļ�ʧ��,session=%d,send_count=%d,�Ѿ������ļ�����%d,��ǰ�ļ�����%d",
						send_header.session_id,
						send_header.send_count,
						itor->second->receive_block,
						header->block_offset);
				return false;
			}
			else if (result == 1)
			{
				//WriteLog("���յ��ļ��ĵ�%d��,���͵�Ϊ%d��", itor->second->receive_block, header->block_offset);
				return true;
			}
		}
		else if (header->block_offset == -1)
		{
			// ���Ͷ����·����ļ��������Ƿ��Ͷ�û�н��յ��ͻ��˷��͵�ack��
			// ���߷��Ͷ��յ��˽��ն˷��͵��ļ������
			FileSaverUnit *unit = file_map[header->session_id];
			delete unit;
			int file_length = ntohl(*(int *) (buffer + sizeof(PacketHeader)));
			int block_num = ntohl(*(int *) (buffer + sizeof(PacketHeader) + 4));
			// ��ȡ�ļ������˴�������ڴ���SaveFileUnit��������������ͷ�
			int filename_length = header->data_length - 8;
			unit = new FileSaverUnit();
			unit->create(file_length, block_num, buffer + sizeof(PacketHeader) + 8,
					filename_length);
			file_map[header->session_id] = unit;
			WriteLog("���½��յ��ļ�����%d���ļ�����%d���ļ�", file_length, block_num);
		}
	}
	return true;
}
