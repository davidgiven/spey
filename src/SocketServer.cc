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

SocketServer::SocketServer()
{
	_fd = -1;
}

SocketServer::~SocketServer()
{
	deinit();
}

void SocketServer::init(SocketAddress& address)
{
	deinit();

	_address = address;
	_fd = socket(PF_INET, SOCK_STREAM, 0);
	if (_fd == -1)
		throw NetworkException("Error creating server socket", errno);
	DetailLog() << "master socket created on fd "
		    << _fd;

	int i = 1;
	setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(i));

	if (_address.bindto(_fd) == -1)
		throw NetworkException("Error binding server socket", errno);
	if (listen(_fd, 5) == -1)
		throw NetworkException("Error listening to server socket", errno);
}

void SocketServer::deinit()
{
	if (_fd != -1)
	{
		DetailLog() << "closing master socket "
			    << _fd;
		close(_fd);
		_fd = -1;
	}
}

int SocketServer::accept(SocketAddress* address)
{
	/* Wait for an incoming socket. */

	sockaddr_in sa;
	socklen_t sas = sizeof(sa);

	int fd;
	{
		Threadlet::Concurrent c;
		
		fd = ::accept(_fd, (sockaddr*) &sa, &sas);
	}

	if (fd == -1)
		throw NetworkException("accept() failed", errno);
	address->set(sa);
	return fd;
}
