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
	SocketServer(SocketAddress& sa);
	~SocketServer();

	Socket accept();

private:
	SocketAddress address;
	int fd;
};

#endif

/* Revision history
 * $Log$
 */