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

	// 发送文件
	if (filename != NULL)
	{
		if (strlen(filename) != 0)
		{
			// 初始化写网卡类
			g_netcard_writer = new NetcardWriter();
			g_file_sender = new FileSender();
			g_file_sender->add_send_file(filename);
		}
		else
		{
			WriteLog("请输入正确参数");
			return 1;
		}
	}
	else
	{
		// 初始化写网卡类
		g_netcard_writer = new NetcardWriter();
		g_file_sender = new FileSender();
	}

	// 初始化读网卡类
	g_netcard_reader = new NetcardReader();
	g_netcard_reader->receive();

	// 主线程循环
	while (1)
	{
		Sleep(1000);
		WriteLog("main函数循环一次");
	}
}
