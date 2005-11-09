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

/* Create a new, uninitialised socket. */

Socket::Socket()
{
	_fd = -1;
	_timeout = 0;
}

/* Destroy an existing socket. */

Socket::~Socket()
{
	deinit();
}

/* Initialise socket from the given file descriptor. */

void Socket::init(int fd)
{
	deinit();
	_fd = fd;
	_timeout = 0;

	DetailLog() << "new explicit slave connection on "
		    << fd;
}

/* Initialise the socket from the given file descriptor, but also mark it as
 * coming from the specified address. */

void Socket::init(int fd, SocketAddress& address)
{
	deinit();

	DetailLog() << "new explicit incoming connection on "
		    << fd
		    << " marked as being from "
		    << address;
	_fd = fd;
	_address = address;
}

/* Initialise the socket to be an outgoing connection to the specified address.
 * */

void Socket::init(SocketAddress& address)
{
	deinit();

	_fd = socket(PF_INET, SOCK_STREAM, 0);
	if (_fd == -1)
		throw NetworkException("Error creating outbound socket", errno);
	DetailLog() << "outbound socket created on fd "
		    << _fd
		    << " to "
		    << address;

	if (address.connectto(_fd) != 0)
	{
		close(_fd);
		_fd = -1;
		throw NetworkException("Error connecting to remote server", errno);
	}
}

void Socket::deinit()
{
	if (_fd != -1)
	{
		close(_fd);
		_fd = -1;
	}
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
		p.fd = _fd;
		p.events = POLLIN | POLLERR | POLLHUP | POLLPRI;
		if (poll(&p, 1, 0) != 0)
			break;

		/* No data. Deschedule. */

		int delay = timeout - now();
		if (delay < 0)
			throw NetworkTimeoutException();

		Threadlet::addrdfd(_fd);
		Threadlet::current()->deschedule(delay);
		Threadlet::subrdfd(_fd);
	}

	return ::read(_fd, buffer, buflength);
}

/* Write data to the socket. */

int Socket::write(void* buffer, int buflength)
{
	/* Wait for the socket becoming writable. */

	for (;;) {
		struct pollfd p;
		p.fd = _fd;
		p.events = POLLOUT | POLLERR | POLLHUP;
		if (poll(&p, 1, 0) != 0)
			break;

		/* No data. Deschedule. */

		Threadlet::addwrfd(_fd);
		Threadlet::current()->deschedule();
		Threadlet::subwrfd(_fd);
	}
	return ::write(_fd, buffer, buflength);
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
		
		/* Convert \n into \r\n. */
		
		if (c == 10)
		{
			c = 13;
			if (write(&c, 1) == -1)
				goto eof;
			c = 10;
		}
		
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
 * Revision 1.8  2005/11/09 00:02:43  dtrg
 * Fixed an issue where bare linefeeds were being sent to the internal server
 * as part of spey's synthesised Received line. This was causing
 * standard-checking MTAs such as qmail to die.
 *
 * Revision 1.7  2004/11/18 17:57:20  dtrg
 * Rewrote logging system so that it no longer tries to subclass stringstream,
 * that was producing bizarre results on gcc 3.3. Added version tracking to
 * the makefile; spey now knows what version and build number it is, and
 * displays the information in the startup banner. Now properly ignores
 * SIGPIPE, which was causing intermittent silent aborts.
 *
 * Revision 1.6  2004/06/30 20:18:49  dtrg
 * Changed the way sockets are initialised; instead of doing it from the
 * Socket and SocketServer constructors, they're set up as zombies and
 * initialised later with an init() method. This is cleaner, and also allows a
 * cunning new feature: the connection to the downstream SMTP server is now
 * only made once the first valid SMTP command is received from the upstream
 * SMTP server. This means that connections are only made once we're
 * reasonably sure that there's going to be a valid SMTP conversation, which
 * should harden spey against DoS attacks like the ones I get every so often.
 * Also took the opportunity to convert more this->blah instance variables
 * into _blah.
 *
 * Revision 1.5  2004/06/09 18:40:34  dtrg
 * Fixed some tracing where the address of incoming connections was being
 * reported incorrectly in the logs (but correctly in the Received lines of
 * incoming messages).
 *
 * Revision 1.4  2004/06/08 19:58:04  dtrg
 * Fixed a bug where the address of incoming connections was thought to be the
 * address of *this* end of the connection, not the other end. In the process,
 * changed some this->blah instance variables to _blah.
 *
 * Revision 1.3  2004/05/30 01:55:13  dtrg
 * Numerous and major alterations to implement a system for processing more
 * than one message at a time, based around coroutines. Fairly hefty
 * rearrangement of constructors and object ownership semantics. Assorted
 * other structural modifications.
 *
 * Revision 1.2  2004/05/14 21:28:22  dtrg
 * Added the ability to create a Socket from a raw file descriptor (needed for
 * inetd mode, where we're going to have a socket passed to us on fd 0).
 *
 * Revision 1.1  2004/05/01 12:20:20  dtrg
 * Initial version.
 */
