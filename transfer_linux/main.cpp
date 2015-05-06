#include <unistd.h>
#include <pthread.h>
#include <string.h>

#include "file_sender.h"
#include "netcard_reader.h"
#include "public_func.h"
#include "public_var.h"
#include "netcard_reader_raw.h"

int main(int argc, char *argv[])
{
	char *filename = NULL;
	int result;
	while ((result = getopt(argc, argv, "f:")) != -1)
	{
		switch (result)
		{
			case 'f' :
				filename = optarg;
				break;
			default :
				break;
		}
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
	pthread_t netcard_reader_t;
	if (pthread_create(&netcard_reader_t, NULL, netcard_reader_thread, NULL) != 0)
	{
		WriteLog("read netcard thread start faild");
		_exit(-1);
	}
	pthread_detach(netcard_reader_t);

	// 主线程循环
	while (1)
	{
		sleep(1);
		WriteLog("main函数循环一次");
	}
}
