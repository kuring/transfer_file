#include "stdafx.h"

#include "session.h"

Session::Session() : session_id(0)
{
	CreateMutex(NULL, FALSE, NULL);
}


Session::~Session()
{
	CloseHandle(mutex);
}

unsigned int Session::alloc_sessionid()
{
	WaitForSingleObject(mutex, INFINITE);
	session_id++;
	ReleaseMutex(mutex);
	return session_id;
}

