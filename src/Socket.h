/* Socket.h
 * Wrapper around a generic TCP socket.
 *
 * Copyright (C) 2004 David Given
 * You may distribute under the terms of the GNU General Public
 * License version 2 as specified in the file COPYING that comes with the
 * Spey distribution.
 *
 * $Source$
 * $State$
 */

#ifndef SOCKET_H
#define SOCKET_H

struct SocketServer;

struct Socket {
	Socket(int fd);
	Socket(SocketAddress& address);
	Socket(int fd, SocketAddress& address);
	~Socket();

	int read(void* buffer, int buflength);
	int write(void* buffer, int buflength);
	string readline();
	void writeline(string l);

	void setaddress(const SocketAddress& address) { _address = address; }
	const SocketAddress& getaddress() { return _address; }
	int timeout() { return _timeout; }
	void timeout(int t) { _timeout = t; }

	int getfd() { return fd; }

protected:
	SocketAddress _address;
	int fd;
	int _timeout;
};

#endif

/* Revision history
 * $Log$
 * Revision 1.4  2004/05/30 01:55:13  dtrg
 * Numerous and major alterations to implement a system for processing more than
 * one message at a time, based around coroutines. Fairly hefty rearrangement of
 * constructors and object ownership semantics. Assorted other structural
 * modifications.
 *
 * Revision 1.3  2004/05/14 23:11:44  dtrg
 * Added decent relaying support. Also converted SocketAddress to use references a
 * lot rather than pass-by-value, out of general tidiness and the hope that it
 * will improve performance a bit.
 *
 * Revision 1.2  2004/05/14 21:28:22  dtrg
 * Added the ability to create a Socket from a raw file descriptor (needed for
 * inetd mode, where we're going to have a socket passed to us on fd 0).
 *
 * Revision 1.1  2004/05/01 12:20:20  dtrg
 * Initial version.
 */
