/* SocketServer.cc
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

#include "spey.h"
#include <errno.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <unistd.h>

SocketServer::SocketServer(SocketAddress& address)
{
	_address = address;
	_fd = socket(PF_INET, SOCK_STREAM, 0);
	if (_fd == -1)
		throw NetworkException("Error creating server socket", errno);
	DetailLog() << "master socket created on fd "
		    << _fd
		    << flush;

	int i = 1;
	setsockopt(_fd, IPPROTO_TCP, SO_REUSEADDR, &i, sizeof(i));

	if (_address.bindto(_fd) == -1)
		throw NetworkException("Error binding server socket", errno);
	if (listen(_fd, 5) == -1)
		throw NetworkException("Error listening to server socket", errno);
}

SocketServer::~SocketServer()
{
	DetailLog() << "closing master socket "
		    << _fd
		    << flush;
	close(_fd);
}

int SocketServer::accept(SocketAddress* address)
{
	/* Wait for an incoming socket. */

	for (;;) {
		struct pollfd p;
		p.fd = _fd;
		p.events = POLLIN | POLLERR | POLLHUP | POLLPRI;
		if (poll(&p, 1, 0) != 0)
			break;

		/* No data. Deschedule. */

		Threadlet::addrdfd(_fd);
		Threadlet::current()->deschedule();
		Threadlet::subrdfd(_fd);
	}

	sockaddr_in sa;
	socklen_t sas = sizeof(sa);
	int fd = ::accept(_fd, (sockaddr*) &sa, &sas);
	if (fd == -1)
		throw NetworkException("accept() failed", errno);
	address->set(sa);
	return fd;
}

/* Revision history
 * $Log$
 * Revision 1.2  2004/05/30 01:55:13  dtrg
 * Numerous and major alterations to implement a system for processing more than
 * one message at a time, based around coroutines. Fairly hefty rearrangement of
 * constructors and object ownership semantics. Assorted other structural
 * modifications.
 *
 * Revision 1.1  2004/05/01 12:20:20  dtrg
 * Initial version.
 */
