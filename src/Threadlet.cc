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
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <list>

enum {
	StackSize = 1 * 1024 * 1024
};

static ucontext rootcontext;
static fd_set readfds;
static fd_set writefds;
static int maxfd = 0;
static list<Threadlet*> processes;
static Threadlet* currentprocess;
static int timeout;

#define foreach(_collection, _iterator) \
        for (typeof((_collection).begin()) _iterator = (_collection).begin(); \
                (_iterator) != (_collection).end(); (_iterator)++)

void Threadlet::trampoline(Threadlet* threadlet)
{
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
	threadlet->_running = 0;
}

Threadlet::Threadlet()
{
	getcontext(&_context);

	_context.uc_stack.ss_sp = mmap(NULL, StackSize,
		PROT_READ | PROT_WRITE | PROT_EXEC,
		MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, 0, 0);
	if (_context.uc_stack.ss_sp == MAP_FAILED)
	{
		int e = errno;
		close(_stackfd);
		_stackfd = -1;
		throw IOException("couldn't map coroutine stack block", e);
	}

	_context.uc_stack.ss_size = StackSize;
	_context.uc_stack.ss_flags = 0;
	_context.uc_link = &rootcontext;

	makecontext(&_context, (void(*)()) Threadlet::trampoline, 1, this);
	_running = 1;
}

Threadlet::~Threadlet()
{
	if (_context.uc_stack.ss_sp)
		munmap(_context.uc_stack.ss_sp, _context.uc_stack.ss_size);
}

/* ======================================================================= */
/*                         THREADLET INTERFACE                             */
/* ======================================================================= */

int Threadlet::debugid()
{
	return -1;
}
	
void Threadlet::deschedule(int delay)
{
	if (timeout > delay)
		timeout = delay;

	currentprocess = NULL;
	swapcontext(&_context, &rootcontext);
}

/* ======================================================================= */
/*                        ROOT PROCESS INTERFACE                           */
/* ======================================================================= */

void Threadlet::invoke()
{
	if (!_running)
		throw InternalException("Tried to invoke dead threadlet");

	currentprocess = this;
	swapcontext(&rootcontext, &_context);
}

/* ======================================================================= */
/*                               SCHEDULER                                 */
/* ======================================================================= */

Threadlet* Threadlet::current()
{
	return currentprocess;
}
	
void Threadlet::addthreadlet(Threadlet* t)
{
	processes.push_back(t);
}

void Threadlet::addrdfd(int fd)
{
	FD_SET(fd, &readfds);
	fd++;
	if (fd > maxfd)
		maxfd = fd;
}

void Threadlet::subrdfd(int fd)
{
	FD_CLR(fd, &readfds);
}
	
void Threadlet::addwrfd(int fd)
{
	FD_SET(fd, &writefds);
	fd++;
	if (fd > maxfd)
		maxfd = fd;
}

void Threadlet::subwrfd(int fd)
{
	FD_CLR(fd, &writefds);
}
	
/* The scheduler is dead simple: it waits for something interesting to happen on
 * a file descriptor, and then gives every threadlet a chance to execute. It could
 * be easily optimised to keep track of what threadlet is interested in what
 * file descriptors and only run those, but there's not really a lot of point:
 * in normal use, there'll be a fairly small number of threadlets running.
 *
 * The only interesting point is that when startScheduler() is called, all
 * threadlets get one chance to run. This is the point at which they should
 * register what file descriptors they're interested in. If nothing registers
 * any interest, nothing will happen --- ever! */

void Threadlet::startScheduler()
{
	for (;;) {
		/* Give all threadlets some CPU time. */

		timeout = INT_MAX;
		foreach (processes, i)
		{
			Threadlet* t = *i;
			if (t->_running)
				t->invoke();
		}

		/* Clean up any terminated threadlets. */

	restartloop:
		foreach (processes, i)
		{
			Threadlet* t = *i;
			if (!t->_running)
			{
				ThreadLog() << "destoying threadlet "
				            << t->debugid();
				processes.remove(t);
				delete t;
				goto restartloop;
			}
		}

		/* Any processes left? */

		if (processes.empty())
			break;

		/* Wait for the next interesting event. */

		fd_set reads = readfds;
		fd_set writes = writefds;

		struct timeval tv;
		tv.tv_sec = timeout / 1000;
		tv.tv_usec = (timeout % 1000) * 1000;
		ThreadLog() << "waiting for "
		            << timeout
			    << "ms";
		select(maxfd, &reads, &writes, NULL, &tv);
	}
}

/* Revision history
 * $Log$
 * Revision 1.1  2004/05/30 01:55:13  dtrg
 * Numerous and major alterations to implement a system for processing more than
 * one message at a time, based around coroutines. Fairly hefty rearrangement of
 * constructors and object ownership semantics. Assorted other structural
 * modifications.
 *
 */

