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
	// 清空file_map中的内容
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
			// 获取文件名，此处申请的内存在SaveFileUnit类的析够函数中释放
			int filename_length = header->data_length - 8;
			FileSaverUnit *unit = new FileSaverUnit();
			unit->create(file_length, block_num, buffer + sizeof(PacketHeader) + 8, filename_length);
			file_map[header->session_id] = unit;
			WriteLog("接收端接收到文件长度%d，文件块数%d的文件", file_length, block_num);
		}
	}
	else
	{
		if (header->block_offset > 0)	// 文件内容块
		{
			std::map<unsigned int, FileSaverUnit *>::iterator itor = file_map.find(header->session_id);
			if (itor == file_map.end())
			{
				return false;
			}
			int result = itor->second->save(buffer);
			if (result == 2)
			{
				// 将文件信息从map中删除
				file_map.erase(itor);
				// 文件接收成功
				PacketHeader send_header;
				send_header.session_id = header->session_id;
				send_header.type = FILE_ACK;
				send_header.send_count = header->send_count;
				g_netcard_writer->add_send_queue((unsigned char *)&send_header, sizeof(PacketHeader));
				WriteLog("接收端向发送端发送文件接收成功");
				return true;
			}
			else if (result == -1)
			{
				// 文件数据块接收失败，可能接收的文件数据块中间丢包了
				PacketHeader send_header;
				send_header.session_id = header->session_id;
				send_header.send_count = header->send_count;
				send_header.block_offset = 0;
				send_header.type = FILE_WRONG;
 				g_netcard_writer->add_send_queue((unsigned char *)&send_header, sizeof(PacketHeader));
 				WriteLog("接收文件失败,session=%d,send_count=%d,已经接收文件块数%d,当前文件块数%d",
						send_header.session_id,
						send_header.send_count,
						itor->second->receive_block,
						header->block_offset);
				return false;
			}
			else if (result == 1)
			{
				//WriteLog("接收到文件的第%d块,发送的为%d块", itor->second->receive_block, header->block_offset);
				return true;
			}
		}
		else if (header->block_offset == -1)
		{
			// 发送端重新发送文件，可能是发送端没有接收到客户端发送的ack包
			// 或者发送端收到了接收端发送的文件错误包
			FileSaverUnit *unit = file_map[header->session_id];
			delete unit;
			int file_length = ntohl(*(int *) (buffer + sizeof(PacketHeader)));
			int block_num = ntohl(*(int *) (buffer + sizeof(PacketHeader) + 4));
			// 获取文件名，此处申请的内存在SaveFileUnit类的析够函数中释放
			int filename_length = header->data_length - 8;
			unit = new FileSaverUnit();
			unit->create(file_length, block_num, buffer + sizeof(PacketHeader) + 8,
					filename_length);
			file_map[header->session_id] = unit;
			WriteLog("重新接收到文件长度%d，文件块数%d的文件", file_length, block_num);
		}
	}
	return true;
}
