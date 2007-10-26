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
#include <list>

static pthread_mutex_t cpulock = PTHREAD_MUTEX_INITIALIZER;
static pthread_key_t selfkey = NULL;

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
	
	int e = pthread_setspecific(selfkey, threadlet);
	if (e)
	{
		SystemLog() << "Unable to set up new threadlet!";
		return NULL;
	}
	
	/* Threadlet code runs with the CPU lock held. */
	
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

Threadlet::Threadlet()
{
	/* Create the underlying pthread for this threadlet. */
	
	int e = pthread_create(&_thread, NULL, Threadlet::trampoline, this);
	if (e)
		throw IOException("couldn't create new thread", e);
}

Threadlet::~Threadlet()
{
	pthread_detach(_thread);
}

/* ======================================================================= */
/*                         THREADLET INTERFACE                             */
/* ======================================================================= */

int Threadlet::releaseCPUlock()
{
	return pthread_mutex_unlock(&cpulock);
}

int Threadlet::takeCPUlock()
{
	return pthread_mutex_lock(&cpulock);
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
	if (selfkey)
		return (Threadlet*) pthread_getspecific(selfkey);
	else
		return NULL;
}
	
/* Revision history
 * $Log$
 * Revision 1.9  2007/10/24 22:50:56  dtrg
 * Fixed the word wrap in the last CVS comment.
 *
 * Revision 1.8  2007/10/24 20:44:15  dtrg
 * Did a lot of minor code cleanups and C++ style improvements: uncopyable C++
 * objects are now marked as such and do not have copy constructors, and RAI is
 * used for the threadlet mutex.
 *
 * Revision 1.7  2007/04/18 22:37:16  dtrg
 * Fixed a nasty race condition where the threadlet destructor was
 * being called without the CPU lock held. Removed the call to
 * pthread_mutexattr_setpshared(), because it seems to be
 * unnecessary and also unportable.
 *
 * Revision 1.6  2007/02/10 16:04:29  dtrg
 * Removed some extraneous debugging tracing.
 *
 * Revision 1.5  2007/01/29 23:32:19  dtrg
 * Fixed a compiler warning.
 *
 * Revision 1.4  2007/01/29 23:05:11  dtrg
 * Due to various unpleasant incompatibilities with ucontext, the
 * entire coroutine implementation has been rewritten to use
 * pthreads instead of user-level scheduling. This should make
 * things far more robust and portable, if a bit more heavyweight.
 * It also has the side effect of drastically simplified threadlet code.
 *
 * Revision 1.3  2005/09/25 23:11:35  dtrg
 * Changed some references to '0' and '1' to 'false' and 'true' for clarity.
 * Fixed a problem where if a threadlet was terminated due to an exception,
 * currentprocess wasn't being reset; this was causing the logging system to
 * occasionally crash when it tried to determine the ID of the current threadlet.
 *
 * Revision 1.2  2004/11/18 17:57:20  dtrg
 * Rewrote logging system so that it no longer tries to subclass stringstream,
 * that was producing bizarre results on gcc 3.3. Added version tracking to the
 * makefile; spey now knows what version and build number it is, and displays the
 * information in the startup banner. Now properly ignores SIGPIPE, which was
 * causing intermittent silent aborts.
 *
 * Revision 1.1  2004/05/30 01:55:13  dtrg
 * Numerous and major alterations to implement a system for processing more than
 * one message at a time, based around coroutines. Fairly hefty rearrangement of
 * constructors and object ownership semantics. Assorted other structural
 * modifications.
 */

