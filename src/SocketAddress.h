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

inline ostream& operator << (ostream& s, SocketAddress& sa)
{
	s << (string) sa;
	return s;
}

#endif

/* Revision history
 * $Log$
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

