/* Socket.cc
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

#include "spey.h"
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <time.h>

Socket::Socket(int fd):
	address(fd)
{
	this->fd = fd;
	_timeout = 0;

	DetailLog() << "new explicit slave connection from "
		    << address
		    << " on "
		    << fd
		    << flush;
}

Socket::Socket(SocketAddress& address)
{
	this->fd = socket(PF_INET, SOCK_STREAM, 0);
	if (this->fd == -1)
		throw NetworkException("Error creating outbound socket", errno);
	DetailLog() << "outbound socket created on fd "
		    << this->fd
		    << flush;

	if (address.connectto(this->fd) != 0)
	{
		close(this->fd);
		throw NetworkException("Error connecting to remote server", errno);
	}
}

Socket::~Socket()
{
	DetailLog() << "closing socket connection "
		    << fd
		    << flush;
	close(this->fd);
}

/* Return the current time in milliseconds since epoch. */

static uint64_t now()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);

	return ((uint64_t)tv.tv_sec)*1000 + (tv.tv_usec/1000);
}
	
int Socket::read(void* buffer, int buflength)
{
	/* When does the timeout take place? */

	uint64_t timeout = now() + (uint64_t)_timeout*1000;

	/* Wait for incoming data. */

	for (;;) {
		struct pollfd p;
		p.fd = fd;
		p.events = POLLIN | POLLERR | POLLHUP | POLLPRI;
		if (poll(&p, 1, 0) != 0)
			break;

		/* No data. Deschedule. */

		int delay = timeout - now();
		if (delay < 0)
			throw NetworkTimeoutException();

		Threadlet::addrdfd(fd);
		Threadlet::current()->deschedule(delay);
		Threadlet::subrdfd(fd);
	}

	return ::read(fd, buffer, buflength);
}

int Socket::write(void* buffer, int buflength)
{
	/* Wait for the socket becoming writable. */

	for (;;) {
		struct pollfd p;
		p.fd = fd;
		p.events = POLLOUT | POLLERR | POLLHUP;
		if (poll(&p, 1, 0) != 0)
			break;

		/* No data. Deschedule. */

		Threadlet::addwrfd(fd);
		Threadlet::current()->deschedule();
		Threadlet::subwrfd(fd);
	}
	return ::write(fd, buffer, buflength);
}

string Socket::readline()
{
	stringstream s;

	for (;;)
	{
		char c;
		if (read(&c, 1) != 1)
			goto eof;
		
		if (c == 13)
			continue;
		if (c == 10)
			break;

		s << c;
	}

	return s.str();

eof:
	throw NetworkException("Socket unexpectedly closed");
}

void Socket::writeline(string l)
{
	char c;

	for (string::size_type i=0; i<l.length(); i++)
	{
		c = l[i];
		if (write(&c, 1) == -1)
			goto eof;
	}

	c = 13;
	if (write(&c, 1) == -1)
		goto eof;

	c = 10;
	if (write(&c, 1) == -1)
		goto eof;
	return;

eof:
	throw NetworkException("Socket unexpectedly closed");
}

/* Revision history
 * $Log$
 * Revision 1.2  2004/05/14 21:28:22  dtrg
 * Added the ability to create a Socket from a raw file descriptor (needed for
 * inetd mode, where we're going to have a socket passed to us on fd 0).
 *
 * Revision 1.1  2004/05/01 12:20:20  dtrg
 * Initial version.
 */
