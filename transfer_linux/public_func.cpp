#include <stdarg.h>
#include <iostream>
#include <stdio.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/types.h>

#include "public_func.h"
#include "public_var.h"

int get_file_length(FILE *file)
{
	int len = 0;
	int oldPos = 0;

	if (file != NULL)
	{
		oldPos = ftell(file);		//获取原先指针位置
		if (fseek(file, 0, SEEK_END) != 0)
			return -1;

		len = ftell(file);
		fseek(file, oldPos, SEEK_SET);
	}
	else
	{
		return -1;
	}
	return len;
}


int check_sum(const void *buffer, int length)
{
	const unsigned short *buf = (const unsigned short *)buffer;
	unsigned int result = 0;

	while (length > 1) {
		result += *(buf++);
		length  -= sizeof(*buf);
	}
	if (length) result += *(unsigned char*)buf;
	result = (result >> 16) + (result & 0xFFFF);
	result += (result >> 16);
	result = (~result)&0xFFFF;

	return (int)result;
}

void GetCurData(char *curData)
{
	time_t lTime;
	struct tm* tmTime = NULL;
	time(&lTime);
	tmTime = localtime(&lTime);
	strftime(curData, 16, "%Y%m%d",tmTime);
}

bool CheckFolder(const  char *sPathName)
{
	char   DirName[256];
	strcpy(DirName,   sPathName);
	int   i,len   =   strlen(DirName);
	if(DirName[len-1]!='/')
		strcat(DirName,"/");

	len   =   strlen(DirName);
	for(i=1;   i<len;   i++)
	{
		if(DirName[i]=='/')
		{
			DirName[i]   =   0;
			if( access(DirName,   0) !=0  )
			{
				if(mkdir(DirName,0755)==-1)
				{
					perror("mkdir   error");
					return   false;
				}
			}
			DirName[i] = '/';
		}
	}
	return  true;
}

void WriteLog(const char *fm, ...)
{
	int iSize=0;
	char buff[10*1024];
	int i=0;
	memset(buff, 0, sizeof(buff));

	char chTime[32]={0};
	time_t now;
	struct tm  *timenow;
	time(&now);
	timenow = localtime(&now);
	sprintf(chTime,"%04u-%02u-%02u %02u:%02u:%02u",1900+timenow->tm_year,timenow->tm_mon+1,timenow->tm_mday,timenow->tm_hour,timenow->tm_min,timenow->tm_sec);
	i  = sprintf( buff,"%s\t",chTime);

	va_list args;
	va_start( args, fm );
	iSize = vsnprintf( buff+i, sizeof(buff) - (i+1), fm, args );
	va_end( args );

	iSize += i;
	if(iSize<10*1024-2)
	{
		buff[iSize]='\n';
		iSize++;
	}

	std::cout<<buff;
/*
	char curData[16]= {0};
	char fileName[300];

	GetCurData(curData);
	memset(fileName, 0, sizeof(fileName));

	int  j=0;
	j  = sprintf(fileName,"%s",PATH_LOG );
	j += sprintf(fileName + j, "%s", TYPE_LOG );
	CheckFolder(fileName);
	j += sprintf( fileName + j, "/%s_%s%s",TYPE_LOG, curData, ".log");

	FILE* fp = fopen(fileName,"ab");
	if (fp != NULL)
	{
		fwrite(buff, 1, iSize,fp);
		fclose(fp);
	}
	return ;*/
}
