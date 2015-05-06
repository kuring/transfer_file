#ifndef _PUBLIC_FUNC_H_
#define _PUBLIC_FUNC_H_

#include <stdio.h>

// 获取文件长度
int get_file_length(FILE *file);

// 计算数据部分的校验和
int check_sum(const void *buffer, int length);

// 写日志
void WriteLog(const char *fm, ...);

bool CheckFolder(const  char *sPathName);

#endif
