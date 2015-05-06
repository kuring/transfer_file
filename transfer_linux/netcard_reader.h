#ifndef _READ_NETCARD_H_
#define _READ_NETCARD_H_

#include <pcap.h>

#include "file_saver.h"

void pcap_callback(u_char *useless, const struct pcap_pkthdr* pkthdr, const u_char *packet);

class NetcardReader
{
public:
	NetcardReader();
	virtual ~NetcardReader();
    bool receive();
private:
    bool find_device();
    bool open();

public :
    FileSaver *file_saver;

private:
    pcap_t *pcap_read;
    char device_name[20];
};

#endif
