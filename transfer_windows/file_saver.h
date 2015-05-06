#ifndef _SAVE_FILE_H_
#define _SAVE_FILE_H_

#include <map>

#include "file_saver_unit.h"

class FileSaver
{
public:
	FileSaver();
	virtual ~FileSaver();
	bool save(const unsigned char *buffer);

private:
	std::map<unsigned int, FileSaverUnit *> file_map;
};

#endif
