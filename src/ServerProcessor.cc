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
		DetailLog() << "waiting for connection";

		Settings::reload();

		/* Wait for an incoming connection. */

		SocketAddress address;
		int fd = _mastersocket.accept(&address);

		/* Create a threadlet to process the connection. */

		try {
			/* Automatically added to scheduler */
			(void) new MessageProcessor(fd, address);
		} catch (Exception e) {
			MessageLog() << "unable to process connection from "
				<< address << ": " << e;

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
