/* SocketServer.h
 * Wrapper around a generic TCP or UDP master socket.
 *
 * Copyright (C) 2004 David Given
 * You may distribute under the terms of the GNU General Public
 * License version 2 as specified in the file COPYING that comes with the
 * Spey distribution.
 *
 * $Source$
 * $State$
 */

#ifndef SOCKETSERVER_H
#define SOCKETSERVER_H

struct SocketServer {
	SocketServer();
	~SocketServer();

	void init(SocketAddress& sa);
	void deinit();

	int accept(SocketAddress* address);
	int getfd() { return _fd; }

private:
	SocketAddress _address;
	int _fd;
};

#endif

/* Revision history
 * $Log$
 * Revision 1.3  2004/06/08 19:58:04  dtrg
 * Fixed a bug where the address of incoming connections was thought to be the
 * address of *this* end of the connection, not the other end. In the process,
 * changed some this->blah instance variables to _blah.
 *
 * Revision 1.2  2004/05/30 01:55:13  dtrg
 * Numerous and major alterations to implement a system for processing more than
 * one message at a time, based around coroutines. Fairly hefty rearrangement of
 * constructors and object ownership semantics. Assorted other structural
 * modifications.
 *
 * Revision 1.1  2004/05/01 12:20:20  dtrg
 * Initial version.
 *
 */
