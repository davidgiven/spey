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

#ifndef SOCKETSESSION_H
#define SOCKETSESSION_H

struct Socket {
	Socket(int fd, SocketAddress address);
	Socket(int fd);
	Socket(SocketAddress address);
	~Socket();

	int read(void* buffer, int buflength);
	int write(void* buffer, int buflength);
	string readline();
	void writeline(string l);

	SocketAddress getaddress() { return address; }
	int timeout() { return _timeout; }
	void timeout(int t) { _timeout = t; }

protected:
	SocketAddress address;
	int fd;
	int _timeout;
};

#endif

/* Revision history
 * $Log$
 * Revision 1.1  2004/05/01 12:20:20  dtrg
 * Initial version.
 */
