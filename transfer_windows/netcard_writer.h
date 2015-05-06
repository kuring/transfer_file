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
	// 当前包所属的session，用于当接收到FILE_WRONG包时删除发送队列中的内容用
	int session_id;
};

// 向网卡写数据线程，定期从队列中读取数据并发送到网卡
DWORD WINAPI write_netcard_thread(void *);

class NetcardWriter
{
public:
    NetcardWriter();
    virtual ~NetcardWriter();

    /**
     * 添加要发送的数据到发送队列中
     * buffer所指向的内存不需要删除
     */
    void add_send_queue(unsigned char *buffer, int length);

    /** 如果队列中有数据，就将队列中的数据发送到网卡
     *  首先将队列中的数据复制到指针中，然后将队列中的数据复制到指针中
     */
    int send_to_card();

    /**
     * 清空发送队列中session_id的数据单元
     * 如果有session_id的数据单元则一定在队列的头部
     * 因此仅从队列头部开始删除，直到遇到第一个session_id不一致的就停止
     */
    void clear_send_queue(int session_id, uint16_t send_count);

    /**
     * 检测发送队列中第一个包是否还是该session的包
     */
    bool check_send_finish(int session_id);

private:
    bool find_device();
    bool open();

private:
    pcap_t *pcap_write;
    char device_name[20];

    // 向网卡上发送数据的队列
    std::queue<SendQueueUnit *> send_queue;

    // 向网卡上发送数据队列的锁
    HANDLE queue_mutex;

    // 采用双缓冲机制实现发送队列
    std::vector<SendQueueUnit *> send_buffer;

    // 读取数据块发送队列线程
    HANDLE write_netcard_t;
};

#endif
