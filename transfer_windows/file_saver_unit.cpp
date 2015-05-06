#include "stdafx.h"

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

	// ���ڴ��������ļ�������
	this->buffer = new unsigned char[this->file_length];
	memset(this->buffer, 0, this->file_length);
}

int FileSaverUnit::save(const unsigned char *buffer)
{
	PacketHeader *header = (PacketHeader *)buffer;
	if (header->block_offset != receive_block + 1)
	{
		// ˵����ȡ����ʱ����
		return -1;
	}

	if (header->check_sum != check_sum(buffer + sizeof(PacketHeader), header->data_length))
	{
		// У���ʧ��
		WriteLog("���յ��İ�У��ʧ��");
		return -1;
	}

	memcpy(this->buffer + this->receive_length, buffer + sizeof(PacketHeader), header->data_length);
	this->receive_block++;
	this->receive_length += header->data_length;

	// �����ļ���Ϻ󱣴��ļ�
	if (this->receive_block == block_num)
	{
		std::string new_filename;
		new_filename = filename;
		new_filename += ".recv";
		// ����Ŀ¼
		std::string path = new_filename.substr(0, new_filename.find_last_of("/"));
		CheckFolder(path.c_str());
		FILE *file = fopen(new_filename.c_str(), "wb+");
		fwrite(this->buffer, this->file_length, 1, file);
		fclose(file);
		return 2;
	}
	return 1;
}
