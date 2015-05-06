#ifndef _WRITE_NETCARD_H_
#define _WRITE_NETCARD_H_

#include <pcap.h>
#include <queue>
#include <vector>
#include <map>
#include "transfer_type.h"


class SendQueueUnit
{
public:
	~SendQueueUnit()
	{
		if (buffer != NULL)
		{
			delete[] buffer;
		}
		buffer = NULL;
	}

public:
	unsigned char *buffer;
	int length;
	// ��ǰ��������session�����ڵ����յ�FILE_WRONG��ʱɾ�����Ͷ����е�������
	int session_id;
};

// ������д�����̣߳����ڴӶ����ж�ȡ���ݲ����͵�����
DWORD WINAPI write_netcard_thread(void *);

class NetcardWriter
{
public:
    NetcardWriter();
    virtual ~NetcardWriter();

    /**
     * ���Ҫ���͵����ݵ����Ͷ�����
     * buffer��ָ����ڴ治��Ҫɾ��
     */
    void add_send_queue(unsigned char *buffer, int length);

    /** ��������������ݣ��ͽ������е����ݷ��͵�����
     *  ���Ƚ������е����ݸ��Ƶ�ָ���У�Ȼ�󽫶����е����ݸ��Ƶ�ָ����
     */
    int send_to_card();

    /**
     * ��շ��Ͷ�����session_id�����ݵ�Ԫ
     * �����session_id�����ݵ�Ԫ��һ���ڶ��е�ͷ��
     * ��˽��Ӷ���ͷ����ʼɾ����ֱ��������һ��session_id��һ�µľ�ֹͣ
     */
    void clear_send_queue(int session_id, uint16_t send_count);

    /**
     * ��ⷢ�Ͷ����е�һ�����Ƿ��Ǹ�session�İ�
     */
    bool check_send_finish(int session_id);

private:
    bool find_device();
    bool open();

private:
    pcap_t *pcap_write;
    char device_name[20];

    // �������Ϸ������ݵĶ���
    std::queue<SendQueueUnit *> send_queue;

    // �������Ϸ������ݶ��е���
    HANDLE queue_mutex;

    // ����˫�������ʵ�ַ��Ͷ���
    std::vector<SendQueueUnit *> send_buffer;

    // ��ȡ���ݿ鷢�Ͷ����߳�
    HANDLE write_netcard_t;
};

#endif
