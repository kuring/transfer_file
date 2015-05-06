#ifndef NETCARD_READER_RAW_H_
#define NETCARD_READER_RAW_H_

#include <sys/types.h>
#include <sys/socket.h>

#include "file_saver.h"

void * netcard_reader_thread(void *);

class NetcardReaderRaw {
public:
	NetcardReaderRaw();
	virtual ~NetcardReaderRaw();
	bool receive();

private:
	bool open();
	bool find_device();
	bool deal_raw_packet(const unsigned char *buffer, int size);

public :
    FileSaver *file_saver;

private:
	int fd;
	char device_name[20];
};

#endif /* NETCARD_READER_RAW_H_ */
