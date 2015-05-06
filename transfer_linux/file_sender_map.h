#ifndef _FILE_SENDER_MAP_H_
#define _FILE_SENDER_MAP_H_

#include <map>
#include <pthread.h>
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
	int fail_count;			// 接收到接收端文件发送失败次数
	int send_count;			// 文件发送次数，将文件加入发送队列中时加1
	time_t last_send_time;	// 上一次发送时间
	std::string filename;	// 发送的文件名
};


// 定期检测FileSenderMap线程
void *check_file_sender_map_thread(void *);

// 用来保存发送完成的文件，等待收到接收端的ACK即可将文件从map中删除
// 如果收到接收端的FILE_WRONG命令，说明接收端接收文件出错，需要重新将文件加入到发送队列中
class FileSenderMap
{
public:
	FileSenderMap();
	~FileSenderMap();

public:
	void add_map(int session_id, FileSenderUnit* unit);

	/**
	 * 对发送失败的文件进行处理
	 * 如果文件发送失败次数在3次以内，重新发送文件
	 * 如果文件发送失败次数超过3次停止发送
	 * 需要通过sessionid和send_count方可唯一确认一次发送
	 */
	void deal_send_fail(int session_id, int send_count);

	// 处理ack数据包
	void deal_ack(int session_id);

	// 定期将map中过期的文件重新发送
	// 如果文件发送失败次数超过3次，则直接从map中删除
	void check_map();

private:
	std::map<int, FileSenderUnit *> map;
	pthread_mutex_t map_mutex;

	// 定期检测发送完成文件map线程
	pthread_t check_file_sender_map_t;
};

#endif
