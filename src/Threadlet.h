/* Threadlet.h
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

#ifndef THREADLET_H
#define THREADLET_H

#include <pthread.h>

struct Threadlet : uncopyable
{
	static void initialise();
	Threadlet();
	virtual ~Threadlet();

	// Called by the threadlet

	struct Concurrent {
		Concurrent() { Threadlet::releaseCPUlock(); }
		~Concurrent() { Threadlet::takeCPUlock(); }
	};
	
	static int takeCPUlock();
	static int releaseCPUlock();
	
	virtual int debugid();
	virtual void run() = 0;

	static int halt();
	
	// Called by the root process
	
	static void* trampoline(void* user);
	void invoke();
	
	// Called by anyone
	
	static Threadlet* current();
	
protected:
	pthread_t _thread;
};

#endif

/* Revision history
 * $Log$
 * Revision 1.3  2007/10/24 20:44:15  dtrg
 * Did a lot of minor code cleanups and C++ style improvements: uncopyable C++
 * objects are now marked as such and do not have copy constructors, and RAI is
 * used for the threadlet mutex.
 *
 * Revision 1.2  2007/01/29 23:05:10  dtrg
 * Due to various unpleasant incompatibilities with ucontext, the
 * entire coroutine implementation has been rewritten to use
 * pthreads instead of user-level scheduling. This should make
 * things far more robust and portable, if a bit more heavyweight.
 * It also has the side effect of drastically simplified threadlet code.
 *
 * Revision 1.1  2004/05/30 01:55:13  dtrg
 * Numerous and major alterations to implement a system for processing more than
 * one message at a time, based around coroutines. Fairly hefty rearrangement of
 * constructors and object ownership semantics. Assorted other structural
 * modifications.
 */

