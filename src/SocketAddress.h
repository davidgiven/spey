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

struct SocketAddress
{
	SocketAddress();
	SocketAddress(int fd);
	SocketAddress(const sockaddr_in& sa);
	SocketAddress(const string& name, int port);
	SocketAddress(const string& name);
	~SocketAddress();

	void setname(const string& name);
	void setport(int port);
	void set(const string& name);
	void set(const sockaddr_in& address);

	int connectto(int fd);
	int bindto(int fd);
	int acceptfrom(int fd);

	string getname() const;
	operator string () const;
	operator unsigned int () const;

private:
	sockaddr_in _sa;
};

inline Logger& operator << (Logger& s, SocketAddress& sa)
{
	s << (string) sa;
	return s;
}

#endif
