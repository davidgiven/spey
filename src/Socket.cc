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

/* Create a new incoming socket from the given file descriptor. */

Socket::Socket(int fd):
	_address(fd)
{
	this->fd = fd;
	_timeout = 0;

	DetailLog() << "new explicit slave connection from "
		    << _address
		    << " on "
		    << fd
		    << flush;
}

/* Create a new outgoing socket connected to the specified address. */

Socket::Socket(SocketAddress& address)
{
	this->fd = socket(PF_INET, SOCK_STREAM, 0);
	if (this->fd == -1)
		throw NetworkException("Error creating outbound socket", errno);
	DetailLog() << "outbound socket created on fd "
		    << this->fd
		    << " to "
		    << address
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
	
/* Read data from the socket. */

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

/* Write data to the socket. */

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

/* Read a line of text from the socket; doesn't include the newline (or CRLF)
 * at the end. */

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

/* Write a line of text to the socket; a CRLF is added to the end. */

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
 * Revision 1.3  2004/05/30 01:55:13  dtrg
 * Numerous and major alterations to implement a system for processing more than
 * one message at a time, based around coroutines. Fairly hefty rearrangement of
 * constructors and object ownership semantics. Assorted other structural
 * modifications.
 *
 * Revision 1.2  2004/05/14 21:28:22  dtrg
 * Added the ability to create a Socket from a raw file descriptor (needed for
 * inetd mode, where we're going to have a socket passed to us on fd 0).
 *
 * Revision 1.1  2004/05/01 12:20:20  dtrg
 * Initial version.
 */
