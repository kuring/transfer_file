#ifndef _SEND_FILE_H_
#define _SEND_FILE_H_

#include <iostream>
#include <deque>

#include "netcard_writer.h"
#include "file_sender_map.h"
#include "session.h"

// 定期从发送文件队列中读取要发送的文件，并将文件发送到网卡的线程
DWORD WINAPI send_file_thread(void *p);

class FileSender
{
public :
	FileSender();
	~FileSender();

	// 增加要发送的文件
	bool add_send_file(const char *filename);

	// 将已经发送过的但是发送失败得到文件重新放入发送队列中
	bool add_send_file(FileSenderUnit *unit);

	// 将发送文件队列中的要发送文件取出来发送到网卡上
	void send_file_from_queue();

private:
	// 发送文件到网卡
	int send(FileSenderUnit *unit);

public:
	// 文件发送完成后将信息保存到map中
	FileSenderMap *file_sender_map;

private :
	// 发送文件队列
	std::deque<FileSenderUnit *> file_sender_queue;

	// 发送文件队列使用的锁
	HANDLE queue_mutex;

	// 用来分配session id
	Session session;

	// 线程句柄
	HANDLE send_file_t;
};

#endif
