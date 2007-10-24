/* ServerProcessor.h
 * The top-level server threadlet.
 *
 * Copyright (C) 2004 David Given
 * You may distribute under the terms of the GNU General Public
 * License version 2 as specified in the file COPYING that comes with the
 * Spey distribution.
 *
 * $Source$
 * $State$
 */

#ifndef SERVERPROCESSOR_H
#define SERVERPROCESSOR_H

struct ServerProcessor: Threadlet
{
	ServerProcessor();
	virtual ~ServerProcessor();

	virtual int debugid();
	virtual void run();

protected:
	SocketServer _mastersocket;
};

#endif

/* Revision history
 * $Log$
 * Revision 1.2  2007/10/24 20:44:15  dtrg
 * Did a lot of minor code cleanups and C++ style improvements: uncopyable C++
 * objects are now marked as such and do not have copy constructors, and RAI is
 * used for the threadlet mutex.
 *
 * Revision 1.1  2004/05/30 01:55:13  dtrg
 * Numerous and major alterations to implement a system for processing more than
 * one message at a time, based around coroutines. Fairly hefty rearrangement of
 * constructors and object ownership semantics. Assorted other structural
 * modifications.
 *
 */

