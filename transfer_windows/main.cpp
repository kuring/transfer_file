#include "stdafx.h"

#include <string.h>

#include "file_sender.h"
#include "netcard_reader.h"
#include "public_func.h"
#include "public_var.h"

int main(int argc, char *argv[])
{
	char *filename = NULL;
	if (argc > 1 && strcmp(argv[1], "-f") == 0)
	{
		filename = argv[2];
	}

	// �����ļ�
	if (filename != NULL)
	{
		if (strlen(filename) != 0)
		{
			// ��ʼ��д������
			g_netcard_writer = new NetcardWriter();
			g_file_sender = new FileSender();
			g_file_sender->add_send_file(filename);
		}
		else
		{
			WriteLog("��������ȷ����");
			return 1;
		}
	}
	else
	{
		// ��ʼ��д������
		g_netcard_writer = new NetcardWriter();
		g_file_sender = new FileSender();
	}

	// ��ʼ����������
	g_netcard_reader = new NetcardReader();
	g_netcard_reader->receive();

	// ���߳�ѭ��
	while (1)
	{
		Sleep(1000);
		WriteLog("main����ѭ��һ��");
	}
}
