#include "file_sender_map.h"
#include "public_func.h"
#include "public_var.h"

void *check_file_sender_map_thread(void *)
{
	while (1)
	{
		sleep(30);
		g_file_sender->file_sender_map->check_map();
	}
	return (void *)1;
}

FileSenderMap::FileSenderMap()
{
	pthread_mutex_init(&map_mutex, NULL);
	// 定期检测发送完成文件map线程
	int result = pthread_create(&check_file_sender_map_t, NULL, check_file_sender_map_thread, this);
	if (result == 0)
	{
		WriteLog("定期检测发送完成文件map线程启动成功");
		pthread_detach(check_file_sender_map_t);
	}
}

FileSenderMap::~FileSenderMap()
{
	pthread_mutex_destroy(&map_mutex);
}

void FileSenderMap::add_map(int session_id, FileSenderUnit *unit)
{
	pthread_mutex_lock(&map_mutex);
	this->map[session_id] = unit;
	pthread_mutex_unlock(&map_mutex);
}

void FileSenderMap::deal_send_fail(int session_id, int send_count)
{
	pthread_mutex_lock(&map_mutex);
	if (this->map.find(session_id) == this->map.end())
	{
		// 未找到文件发送失败的session
		pthread_mutex_unlock(&map_mutex);
		return ;
	}
	if (this->map[session_id]->send_count <= 3)
	{
		// 当收到多余的FILE_WRONG包时，由于send_count已经加1，文件不会重新加入发送队列中
		if (this->map[session_id]->send_count == send_count)
		{
			// 重新发送数据包
			g_file_sender->add_send_file(map[session_id]);
			pthread_mutex_unlock(&map_mutex);
			WriteLog("接收到文件发送失败命令，重新将文件%s加入发送队列,send_count=%d", map[session_id]->filename.c_str(), map[session_id]->send_count);
			return ;
		}
	}
	else
	{
		WriteLog("发送端收到接收端文件%s发送失败超过3次，将文件从发送map中删除", map[session_id]->filename.c_str());
		this->map.erase(session_id);
	}
	pthread_mutex_unlock(&map_mutex);
}

void FileSenderMap::deal_ack(int session_id)
{
	pthread_mutex_lock(&map_mutex);
	if (this->map.find(session_id) == this->map.end())
	{
		// 未找到相关的session_id
		return ;
	}
	FileSenderUnit *unit = this->map[session_id];
	WriteLog("文件%s发送成功", unit->filename.c_str());
	delete unit;
	this->map.erase(session_id);
	pthread_mutex_unlock(&map_mutex);
}

void FileSenderMap::check_map()
{
	pthread_mutex_lock(&map_mutex);
	time_t now;
	time(&now);
	for (std::map<int, FileSenderUnit *>::iterator itor = this->map.begin(); itor != map.end(); itor++)
	{
		FileSenderUnit *unit = itor->second;
		if (now - unit->last_send_time > 60)
		{
			// 检测发送队列中第一个包是否还是该session的包
			// 如果是说明还没有发送完毕
			if (!g_netcard_writer->check_send_finish(unit->session_id))
			{
				continue;
			}

			if (unit->send_count < 3)
			{
				// 最后发送包时间已经超过60秒，并且发送文件次数少于3次，重新发送文件
				g_file_sender->add_send_file(map[itor->first]);
				WriteLog("文件%s发送失败，开始第%d次发送", unit->filename.c_str(), unit->send_count);
			}
			else
			{
				// 将文件从map中删除
				WriteLog("文件%s发送失败，已经发送%d次，不再发送", unit->filename.c_str(), unit->send_count);
				this->map.erase(itor->first);
				delete unit;
			}
		}
	}
	pthread_mutex_unlock(&map_mutex);
}
