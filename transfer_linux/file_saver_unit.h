#ifndef _SAVE_FILE_UNIT_H_
#define _SAVE_FILE_UNIT_H_

class FileSaverUnit
{
public:
	FileSaverUnit();
	virtual ~FileSaverUnit();

	// 接收到第一个文件包，创建文件，并保存文件的相关信息
	// filename字段不包括字符串最后的'\0'
	void create(int file_length, int block_num, const unsigned char *filename, int filename_length);

	int save(const unsigned char *buffer);

public:
	int file_length;			// 文件总大小
	int block_num;				// 文件的总块数，一个块的大小为BLOCK_SIZE，文件尾部的不满BLOCK_SIZE按一个计算
	int receive_block;			// 已经接收的文件块数
	int receive_length;			// 已经接收的文件长度
	unsigned char *buffer;		// 文件在内存中的缓冲区
	char *filename;				// 要接收的文件名
};

#endif
