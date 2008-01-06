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

struct SocketServer : uncopyable
{
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
