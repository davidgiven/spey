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
#include <errno.h>
#include <unistd.h>
#include <sys/poll.h>

Socket::Socket(int fd, SocketAddress address)
{
	this->fd = fd;
	this->address = address;
	_timeout = 0;

	DetailLog() << "new slave connection from "
		    << address
		    << " on "
		    << fd
		    << flush;
}

Socket::Socket(SocketAddress address)
{
	this->fd = socket(PF_INET, SOCK_STREAM, 0);
	if (this->fd == -1)
		throw NetworkException("Error creating client socket", errno);
	DetailLog() << "client socket created on fd "
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

int Socket::read(void* buffer, int buflength)
{
	if (_timeout > 0)
	{
		struct pollfd p;
		p.fd = fd;
		p.events = POLLIN | POLLERR | POLLHUP | POLLPRI;
		if (poll(&p, 1, _timeout*1000) == 0)
			throw NetworkTimeoutException();
	}

	return ::read(fd, buffer, buflength);
}

int Socket::write(void* buffer, int buflength)
{
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
 */