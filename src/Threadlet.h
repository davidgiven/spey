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

struct Threadlet {
	static void initialise();
	Threadlet();
	virtual ~Threadlet();

	// Called by the threadlet

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
 * Revision 1.1  2004/05/30 01:55:13  dtrg
 * Numerous and major alterations to implement a system for processing more than
 * one message at a time, based around coroutines. Fairly hefty rearrangement of
 * constructors and object ownership semantics. Assorted other structural
 * modifications.
 */

