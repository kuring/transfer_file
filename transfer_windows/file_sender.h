#ifndef _SEND_FILE_H_
#define _SEND_FILE_H_

#include <iostream>
#include <deque>

#include "netcard_writer.h"
#include "file_sender_map.h"
#include "session.h"

// ���ڴӷ����ļ������ж�ȡҪ���͵��ļ��������ļ����͵��������߳�
DWORD WINAPI send_file_thread(void *p);

class FileSender
{
public :
	FileSender();
	~FileSender();

	// ����Ҫ���͵��ļ�
	bool add_send_file(const char *filename);

	// ���Ѿ����͹��ĵ��Ƿ���ʧ�ܵõ��ļ����·��뷢�Ͷ�����
	bool add_send_file(FileSenderUnit *unit);

	// �������ļ������е�Ҫ�����ļ�ȡ�������͵�������
	void send_file_from_queue();

private:
	// �����ļ�������
	int send(FileSenderUnit *unit);

public:
	// �ļ�������ɺ���Ϣ���浽map��
	FileSenderMap *file_sender_map;

private :
	// �����ļ�����
	std::deque<FileSenderUnit *> file_sender_queue;

	// �����ļ�����ʹ�õ���
	HANDLE queue_mutex;

	// ��������session id
	Session session;

	// �߳̾��
	HANDLE send_file_t;
};

#endif
