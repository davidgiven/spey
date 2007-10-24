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
 * Revision 1.1  2004/05/30 01:55:13  dtrg
 * Numerous and major alterations to implement a system for processing more than
 * one message at a time, based around coroutines. Fairly hefty rearrangement of
 * constructors and object ownership semantics. Assorted other structural
 * modifications.
 *
 */

