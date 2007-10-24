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

/* Revision history
 * $Log$
 * Revision 1.7  2007/04/19 09:44:04  dtrg
 * The socket is now properly marked with SO_REUSEADDR, so
 * hopefully the connection-in-use errors when restarting spey should
 * have gone away.
 *
 * Revision 1.6  2007/01/29 23:05:10  dtrg
 * Due to various unpleasant incompatibilities with ucontext, the
 * entire coroutine implementation has been rewritten to use
 * pthreads instead of user-level scheduling. This should make
 * things far more robust and portable, if a bit more heavyweight.
 * It also has the side effect of drastically simplified threadlet code.
 *
 * Revision 1.5  2004/11/18 17:57:20  dtrg
 * Rewrote logging system so that it no longer tries to subclass stringstream,
 * that was producing bizarre results on gcc 3.3. Added version tracking to the
 * makefile; spey now knows what version and build number it is, and displays the
 * information in the startup banner. Now properly ignores SIGPIPE, which was
 * causing intermittent silent aborts.
 *
 * Revision 1.4  2004/06/30 20:18:49  dtrg
 * Changed the way sockets are initialised; instead of doing it from the Socket
 * and SocketServer constructors, they're set up as zombies and initialised later
 * with an init() method. This is cleaner, and also allows a cunning new feature:
 * the connection to the downstream SMTP server is now only made once the first
 * valid SMTP command is received from the upstream SMTP server. This means that
 * connections are only made once we're reasonably sure that there's going to be a
 * valid SMTP conversation, which should harden spey against DoS attacks like the
 * ones I get every so often. Also took the opportunity to convert more this->blah
 * instance variables into _blah.
 *
 * Revision 1.3  2004/06/08 19:58:04  dtrg
 * Fixed a bug where the address of incoming connections was thought to be the
 * address of *this* end of the connection, not the other end. In the process,
 * changed some this->blah instance variables to _blah.
 *
 * Revision 1.2  2004/05/30 01:55:13  dtrg
 * Numerous and major alterations to implement a system for processing more than
 * one message at a time, based around coroutines. Fairly hefty rearrangement of
 * constructors and object ownership semantics. Assorted other structural
 * modifications.
 *
 * Revision 1.1  2004/05/01 12:20:20  dtrg
 * Initial version.
 */
