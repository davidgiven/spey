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

ServerProcessor::ServerProcessor():
	_mastersocket(FromAddress)
{
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

		int fd = _mastersocket.accept();

		/* Automatically added to scheduler */
		(void) new MessageProcessor(fd);
	}
}

/* Revision history
 * $Log$
 */

