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

struct Threadlet {
	Threadlet();
	virtual ~Threadlet();

	// Called by the threadlet

	virtual int debugid();
	virtual void run() = 0;
	void deschedule(int delay = INT_MAX);

	// Called by the root process
	
	void invoke();

	// Scheduler interface
	
	static Threadlet* current();
	static void addthreadlet(Threadlet* t);
	static void addrdfd(int fd);
	static void subrdfd(int fd);
	static void addwrfd(int fd);
	static void subwrfd(int fd);
	static void startScheduler();

protected:
	int _stackfd;
	struct ucontext _context;
	bool _running;

	static void trampoline(Threadlet* threadlet);
};

#endif

/* Revision history
 * $Log$
 */

