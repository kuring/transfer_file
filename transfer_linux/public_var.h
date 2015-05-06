#ifndef _PUBLIC_VAR_
#define _PUBLIC_VAR_

#include "session.h"
#include "file_sender.h"
#include "netcard_reader.h"
#include "netcard_writer.h"
#include "netcard_reader_raw.h"

// 要发送的一个包中的文件块长度
#define BLOCK_SIZE 1024

#define PATH_LOG "/data/transfer_file/log/"
#define TYPE_LOG "transfer_file"

extern FileSender *g_file_sender;

extern NetcardWriter *g_netcard_writer;

//extern NetcardReader *g_netcard_reader;

extern NetcardReaderRaw *g_netcard_reader;

#endif
