/* Threadlet.cc
 * A generic stripped-down coroutine.
 *
 * Copyright (C) 2004 David Given
 * You may distribute under the terms of the GNU General Public
 * License version 2 as specified in the file COPYING that comes with the
 * Spey distribution.
 *
 * $Source$
 * $State$
 */

#include "spey.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <assert.h>
#include <signal.h>
#include <list>

static pthread_mutex_t cpulock = PTHREAD_MUTEX_INITIALIZER;
static pthread_key_t selfkey;

#define foreach(_collection, _iterator) \
        for (typeof((_collection).begin()) _iterator = (_collection).begin(); \
                (_iterator) != (_collection).end(); (_iterator)++)

void Threadlet::initialise()
{
	/* Create the key used to store which Threadlet object is currently
	 * running. */

	(void) pthread_key_create(&selfkey, NULL);

	/* Take the CPU lock. */

	Threadlet::takeCPUlock();
}

void* Threadlet::trampoline(void* user)
{
	Threadlet* threadlet = (Threadlet*) user;

	/* Ensure we never get any SIGPIPEs, which we don't catch and will
	 * cause an abort. By ignoring them, we ensure that read() and write()
	 * return error codes instead. */

	signal(SIGPIPE, SIG_IGN);

	/* Set up the threadlet state. */

	int e = pthread_setspecific(selfkey, threadlet);
	if (e)
	{
		SystemLog() << "Unable to set up new threadlet!";
		return NULL;
	}

	/* Threadlet code runs with the CPU lock held. */

	DetailLog() << "threadlet " << (intptr_t) threadlet <<
		" init";
	threadlet->takeCPUlock();
	try {
		threadlet->run();
	} catch (NetworkTimeoutException e) {
		Statistics::timeout();
		MessageLog() << "Socket timeout; aborting";
	} catch (NetworkException e) {
		MessageLog() << "exception caught: "
			     << e;
		MessageLog() << "message processing aborted";
	} catch (SQLException e) {
		SystemLog() << "SQL error: "
			     << e;
	} catch (...) {
		SystemLog() << "Uncaught exception in threadlet!";
	}

	/* Finished. Delete the threadlet and exit. Remember that
	 * the destructor must run with the CPU lock held. */

	delete threadlet;
	Threadlet::releaseCPUlock();
	return NULL;
}

Threadlet::Threadlet():
	_thread(NULL)
{
}

Threadlet::~Threadlet()
{
	if (_thread)
		pthread_detach(_thread);
}

void Threadlet::start()
{
	/* Create and start the underlying pthread for this threadlet. */

	DetailLog() << "create thread " << (intptr_t) this;
	int e = pthread_create(&_thread, NULL, Threadlet::trampoline, this);
	if (e)
		throw IOException("couldn't create new thread", e);
	DetailLog() << "thread created";
}

/* ======================================================================= */
/*                         THREADLET INTERFACE                             */
/* ======================================================================= */

int Threadlet::releaseCPUlock()
{
	int e = pthread_mutex_unlock(&cpulock);
	if (e)
		SystemLog() << "pthread_mutex_unlock() failed with " << e;
	return e;
}

int Threadlet::takeCPUlock()
{
	int e = pthread_mutex_lock(&cpulock);
	if (e)
		SystemLog() << "pthread_mutex_lock() failed with " << e;
	return e;
}

int Threadlet::halt()
{
	/* Sleeps forever. */

	Threadlet::releaseCPUlock();
	for (;;)
		pause();
	return 0;
}

int Threadlet::debugid()
{
	return -1;
}

/* ======================================================================= */
/*                               UTILITIES                                 */
/* ======================================================================= */

Threadlet* Threadlet::current()
{
	return (Threadlet*) pthread_getspecific(selfkey);
}
