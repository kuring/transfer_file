#ifndef _PUBLIC_FUNC_H_
#define _PUBLIC_FUNC_H_

#include <stdio.h>

// ��ȡ�ļ�����
int get_file_length(FILE *file);

// �������ݲ��ֵ�У���
int check_sum(const void *buffer, int length);

// д��־
void WriteLog(const char *fm, ...);

bool CheckFolder(const  char *sPathName);

#endif
