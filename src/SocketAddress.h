/* SocketAddress.h
 * Wrapper around a generic IPv4 socket address.
 *
 * Copyright (C) 2004 David Given
 * You may distribute under the terms of the GNU General Public
 * License version 2 as specified in the file COPYING that comes with the
 * Spey distribution.
 *
 * $Source$
 * $State$
 */

#ifndef SOCKETADDRESS_H
#define SOCKETADDRESS_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

struct SocketAddress {
	SocketAddress();
	SocketAddress(string name, int port);
	SocketAddress(string name);
	~SocketAddress();

	void setname(string name);
	void setport(int port);
	void set(string name);

	int connectto(int fd);
	int bindto(int fd);
	int acceptfrom(int fd);

	string name();
	operator string ();
	operator unsigned int ();

private:
	sockaddr_in sa;
};

inline ostream& operator << (ostream& s, SocketAddress& sa)
{
	s << (string) sa;
	return s;
}

#endif

/* Revision history
 * $Log$
 */