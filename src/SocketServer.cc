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
	this->fd = socket(PF_INET, SOCK_STREAM, 0);
	if (this->fd == -1)
		throw NetworkException("Error creating server socket", errno);
	DetailLog() << "master socket created on fd "
		    << this->fd
		    << flush;

	int i = 1;
	setsockopt(this->fd, IPPROTO_TCP, SO_REUSEADDR, &i, sizeof(i));

	if (address.bindto(this->fd) == -1)
		throw NetworkException("Error binding server socket", errno);
	if (listen(this->fd, 5) == -1)
		throw NetworkException("Error listening to server socket", errno);
}

SocketServer::~SocketServer()
{
	DetailLog() << "closing master socket "
		    << fd
		    << flush;
	close(this->fd);
}

int SocketServer::accept()
{
	/* Wait for an incoming socket. */

	for (;;) {
		struct pollfd p;
		p.fd = fd;
		p.events = POLLIN | POLLERR | POLLHUP | POLLPRI;
		if (poll(&p, 1, 0) != 0)
			break;

		/* No data. Deschedule. */

		Threadlet::addrdfd(fd);
		Threadlet::current()->deschedule();
		Threadlet::subrdfd(fd);
	}

	return ::accept(fd, NULL, 0);
}

/* Revision history
 * $Log$
 * Revision 1.1  2004/05/01 12:20:20  dtrg
 * Initial version.
 */
