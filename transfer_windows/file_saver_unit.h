#ifndef _SAVE_FILE_UNIT_H_
#define _SAVE_FILE_UNIT_H_

class FileSaverUnit
{
public:
	FileSaverUnit();
	virtual ~FileSaverUnit();

	// ���յ���һ���ļ����������ļ����������ļ��������Ϣ
	// filename�ֶβ������ַ�������'\0'
	void create(int file_length, int block_num, const unsigned char *filename, int filename_length);

	int save(const unsigned char *buffer);

public:
	int file_length;			// �ļ��ܴ�С
	int block_num;				// �ļ����ܿ�����һ����Ĵ�СΪBLOCK_SIZE���ļ�β���Ĳ���BLOCK_SIZE��һ������
	int receive_block;			// �Ѿ����յ��ļ�����
	int receive_length;			// �Ѿ����յ��ļ�����
	unsigned char *buffer;		// �ļ����ڴ��еĻ�����
	char *filename;				// Ҫ���յ��ļ���
};

#endif
