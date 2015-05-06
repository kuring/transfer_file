#ifndef _SESSION_H_
#define _SESSION_H_


class Session
{
public :
	Session();
	virtual ~Session();
	unsigned int alloc_sessionid();

private :
	unsigned int session_id;
	HANDLE mutex;
};

#endif

