#include "stdafx.h"

#include "file_sender_map.h"
#include "public_func.h"
#include "public_var.h"

DWORD WINAPI check_file_sender_map_thread(void *)
{
	while (1)
	{
		Sleep(30000);
		g_file_sender->file_sender_map->check_map();
	}
	return 1;
}


FileSenderMap::FileSenderMap()
{
	map_mutex = CreateMutex(NULL, FALSE, NULL);
	// ���ڼ�ⷢ������ļ�map�߳�
	DWORD tid;
	if (check_file_sender_map_t = CreateThread(NULL, 0, check_file_sender_map_thread, this, 0, &tid))
	{
		CloseHandle(check_file_sender_map_t);
	}
}

FileSenderMap::~FileSenderMap()
{
	CloseHandle(map_mutex);
}

void FileSenderMap::add_map(int session_id, FileSenderUnit *unit)
{
	WaitForSingleObject(map_mutex, INFINITE);
	this->map[session_id] = unit;
	ReleaseMutex(map_mutex);
}

void FileSenderMap::deal_send_fail(int session_id, int send_count)
{
	WaitForSingleObject(map_mutex, INFINITE);
	if (this->map.find(session_id) == this->map.end())
	{
		// δ�ҵ��ļ�����ʧ�ܵ�session
		ReleaseMutex(map_mutex);
		return ;
	}
	if (this->map[session_id]->send_count <= 3)
	{
		// ���յ������FILE_WRONG��ʱ������send_count�Ѿ���1���ļ��������¼��뷢�Ͷ�����
		if (this->map[session_id]->send_count == send_count)
		{
			// ���·������ݰ�
			g_file_sender->add_send_file(map[session_id]);
			ReleaseMutex(map_mutex);
			WriteLog("���յ��ļ�����ʧ��������½��ļ�%s���뷢�Ͷ���,send_count=%d", map[session_id]->filename.c_str(), map[session_id]->send_count);
			return ;
		}
	}
	else
	{
		WriteLog("���Ͷ��յ����ն��ļ�%s����ʧ�ܳ���3�Σ����ļ��ӷ���map��ɾ��", map[session_id]->filename.c_str());
		this->map.erase(session_id);
	}
	ReleaseMutex(map_mutex);
}

void FileSenderMap::deal_ack(int session_id)
{
	WaitForSingleObject(map_mutex, INFINITE);
	if (this->map.find(session_id) == this->map.end())
	{
		// δ�ҵ���ص�session_id
		return ;
	}
	FileSenderUnit *unit = this->map[session_id];
	WriteLog("�ļ�%s���ͳɹ�", unit->filename.c_str());
	delete unit;
	this->map.erase(session_id);
	ReleaseMutex(map_mutex);
}

void FileSenderMap::check_map()
{
	WaitForSingleObject(map_mutex, INFINITE);
	time_t now;
	time(&now);
	for (std::map<int, FileSenderUnit *>::iterator itor = this->map.begin(); itor != map.end(); itor++)
	{
		FileSenderUnit *unit = itor->second;
		if (now - unit->last_send_time > 60)
		{
			// ��ⷢ�Ͷ����е�һ�����Ƿ��Ǹ�session�İ�
			// �����˵����û�з������
			if (!g_netcard_writer->check_send_finish(unit->session_id))
			{
				continue;
			}

			if (unit->send_count < 3)
			{
				// ����Ͱ�ʱ���Ѿ�����60�룬���ҷ����ļ���������3�Σ����·����ļ�
				g_file_sender->add_send_file(map[itor->first]);
				WriteLog("�ļ�%s����ʧ�ܣ���ʼ��%d�η���", unit->filename.c_str(), unit->send_count);
			}
			else
			{
				// ���ļ���map��ɾ��
				WriteLog("�ļ�%s����ʧ�ܣ��Ѿ�����%d�Σ����ٷ���", unit->filename.c_str(), unit->send_count);
				this->map.erase(itor->first);
				delete unit;
			}
		}
	}
	ReleaseMutex(map_mutex);
}
