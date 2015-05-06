#include <string.h>
#include <stdio.h>
#include <iostream>

#include "file_saver_unit.h"
#include "transfer_type.h"
#include "public_func.h"

FileSaverUnit::FileSaverUnit()
	: file_length(0),
	  block_num(0),
	  receive_block(0),
	  receive_length(0),
	  buffer(NULL),
	  filename(NULL)
{}

FileSaverUnit::~FileSaverUnit()
{
	if (filename != NULL)
	{
		delete filename;
	}
	filename = NULL;

	if (buffer != NULL)
	{
		delete buffer;
	}
	buffer = NULL;
}

void FileSaverUnit::create(int file_length, int block_num, const unsigned char *filename, int filename_length)
{
	this->file_length = file_length;
	this->block_num = block_num;
	this->filename = new char[filename_length + 1];
	memcpy(this->filename, filename, filename_length);
	this->filename[filename_length] = '\0';

	// 在内存中申请文件缓冲区
	this->buffer = new unsigned char[this->file_length];
	memset(this->buffer, 0, this->file_length);
}

int FileSaverUnit::save(const unsigned char *buffer)
{
	PacketHeader *header = (PacketHeader *)buffer;
	if (header->block_offset != receive_block + 1)
	{
		// 说明读取网卡时丢包
		return -1;
	}

	if (header->check_sum != check_sum(buffer + sizeof(PacketHeader), header->data_length))
	{
		// 校验包失败
		WriteLog("接收到的包校验失败");
		return -1;
	}

	memcpy(this->buffer + this->receive_length, buffer + sizeof(PacketHeader), header->data_length);
	this->receive_block++;
	this->receive_length += header->data_length;

	// 接收文件完毕后保存文件
	if (this->receive_block == block_num)
	{
		std::string new_filename;
		new_filename = filename;
		new_filename += ".recv";
		// 创建目录
		std::string path = new_filename.substr(0, new_filename.find_last_of("/"));
		CheckFolder(path.c_str());
		FILE *file = fopen(new_filename.c_str(), "wb+");
		fwrite(this->buffer, this->file_length, 1, file);
		fclose(file);
		return 2;
	}
	return 1;
}
