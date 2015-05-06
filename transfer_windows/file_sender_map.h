#ifndef _FILE_SENDER_MAP_H_
#define _FILE_SENDER_MAP_H_

#include <map>
#include <iostream>
#include <time.h>

class FileSenderUnit
{
public:
	FileSenderUnit()
	{
		fail_count = 0;
		session_id = 0;
		send_count = 0;
		time(&last_send_time);
	}

public:
	int session_id;
	int fail_count;			// ���յ����ն��ļ�����ʧ�ܴ���
	int send_count;			// �ļ����ʹ��������ļ����뷢�Ͷ�����ʱ��1
	time_t last_send_time;	// ��һ�η���ʱ��
	std::string filename;	// ���͵��ļ���
};


// ���ڼ��FileSenderMap�߳�
DWORD WINAPI check_file_sender_map_thread(void *);

// �������淢����ɵ��ļ����ȴ��յ����ն˵�ACK���ɽ��ļ���map��ɾ��
// ����յ����ն˵�FILE_WRONG���˵�����ն˽����ļ�������Ҫ���½��ļ����뵽���Ͷ�����
class FileSenderMap
{
public:
	FileSenderMap();
	~FileSenderMap();

public:
	void add_map(int session_id, FileSenderUnit* unit);

	/**
	 * �Է���ʧ�ܵ��ļ����д���
	 * ����ļ�����ʧ�ܴ�����3�����ڣ����·����ļ�
	 * ����ļ�����ʧ�ܴ�������3��ֹͣ����
	 * ��Ҫͨ��sessionid��send_count����Ψһȷ��һ�η���
	 */
	void deal_send_fail(int session_id, int send_count);

	// ����ack���ݰ�
	void deal_ack(int session_id);

	// ���ڽ�map�й��ڵ��ļ����·���
	// ����ļ�����ʧ�ܴ�������3�Σ���ֱ�Ӵ�map��ɾ��
	void check_map();

private:
	std::map<int, FileSenderUnit *> map;
	HANDLE map_mutex;

	// ���ڼ�ⷢ������ļ�map�߳�
	HANDLE check_file_sender_map_t;
};

#endif
