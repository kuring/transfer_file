#include "session.h"

Session::Session() : session_id(0)
{
	pthread_mutex_init(&mutex, NULL);
}


Session::~Session()
{
	pthread_mutex_destroy(&mutex);
}

unsigned int Session::alloc_sessionid()
{
	pthread_mutex_lock(&mutex);
	session_id++;
	pthread_mutex_unlock(&mutex);
	return session_id;
}

