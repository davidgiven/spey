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
#include <unistd.h>

SocketServer::SocketServer(SocketAddress& address)
{
	this->fd = socket(PF_INET, SOCK_STREAM, 0);
	if (this->fd == -1)
		throw NetworkException("Error creating server socket", errno);
	DetailLog() << "master socket created on fd "
		    << this->fd
		    << flush;

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

Socket SocketServer::accept()
{
	SocketAddress sa;
	int fd = sa.acceptfrom(this->fd);
	return Socket(fd, sa);
}

/* Revision history
 * $Log$
 */