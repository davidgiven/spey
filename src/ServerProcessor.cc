/* ServerProcessor.cc
 * The top-level server threadlet.
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
#include <unistd.h>

ServerProcessor::ServerProcessor()
{
	_mastersocket.init(FromAddress);
	Threadlet::addthreadlet(this);
}

ServerProcessor::~ServerProcessor()
{
}

int ServerProcessor::debugid()
{
	return _mastersocket.getfd();
}

void ServerProcessor::run()
{
	for (;;)
	{
		MessageLog() << "waiting for connection"
			     << flush;

		Settings::reload();

		/* Wait for an incoming connection. */

		SocketAddress address;
		int fd = _mastersocket.accept(&address);

		/* Create a threadlet to process the connection. */

		try {
			/* Automatically added to scheduler */
			(void) new MessageProcessor(fd, address);
		} catch (Exception e) {
			MessageLog() << "unable to process connection: "
				     << e
				     << flush;

			/* fd may or may not have been closed by the
			 * MessageProcessor when it shut down. We make a single
			 * bona-fide attempt to give the appropriate response
			 * and give up. */

			string message = "421 " + Settings::identity() +
				" Service not available, closing" +
				" transmission channel\r\n";
			(void) write(fd, message.c_str(), message.length());
			(void) close(fd);
		}
	}
}

/* Revision history
 * $Log$
 * Revision 1.3  2004/06/28 18:57:56  dtrg
 * Added a fix where spey no longer exits if it cannot contact the downstream mail
 * server.
 *
 * Revision 1.2  2004/06/08 19:58:04  dtrg
 * Fixed a bug where the address of incoming connections was thought to be the
 * address of *this* end of the connection, not the other end. In the process,
 * changed some this->blah instance variables to _blah.
 *
 * Revision 1.1  2004/05/30 01:55:13  dtrg
 * Numerous and major alterations to implement a system for processing more than
 * one message at a time, based around coroutines. Fairly hefty rearrangement of
 * constructors and object ownership semantics. Assorted other structural
 * modifications.
 */

