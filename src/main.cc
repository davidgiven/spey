/* main.cc
 * Main program.
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
#include <iostream>
#include <fstream>

SQL Sql;
SocketAddress FromAddress;
SocketAddress ToAddress;

static int daemonmode(CLI& cli)
{
	// Detach from the console.
	if (!cli.x())
	{
		daemon(0, 0);
		Logger::detach();
	}
	
	Sql.open(cli.d());
	FromAddress.set(cli.f());
	ToAddress.set(cli.t());

	DetailLog() << "------------- STARTUP ----------------";
	DetailLog() << "Spey version " MAJORVERSION " build " BUILDCOUNT;

	SystemLog() << "listening on "
		    << FromAddress;

	/* Automatically added to scheduler */
	(void) new ServerProcessor();

	ofstream("/var/run/spey.pid") << getpid() << endl;

	Threadlet::startScheduler();
	SystemLog() << "scheduler terminated!";
	return 0;
}

static int inetdmode(CLI& cli)
{
	Sql.open(cli.d());
	ToAddress.set(cli.t());

	DetailLog() << "------------- INETD STARTUP ----------------";
	DetailLog() << "Spey version " MAJORVERSION " build " BUILDCOUNT;

	try {
		Settings::reload();
		SocketAddress dummyaddress;
		(void) new MessageProcessor(0, dummyaddress);
		Threadlet::startScheduler();
	} catch (NetworkTimeoutException e) {
		Statistics::timeout();
		MessageLog() << "Socket timeout; aborting";
	} catch (NetworkException e) {
		MessageLog() << "exception caught: "
			     << e;
		MessageLog() << "message processing aborted";
	} catch (SQLException e) {
		SystemLog() << "SQL error: "
			     << e;
	}

	return 0;
}

int main(int argc, char* argv[])
{
	/* Ensure we never get any SIGPIPEs, which we don't catch and will
	 * cause an abort. By ignoring them, we ensure that read() and write()
	 * return error codes instead. */

	signal(SIGPIPE, SIG_IGN);
	
	try {
		CLI cli(argc, argv);
		Logger::setlevel(cli.v());
		
		if (cli.i())
			return inetdmode(cli);
		else
			return daemonmode(cli);
	} catch (string e) {
		SystemLog() << "exception caught: "
			    << e;
		exit(-1);
	} catch (Exception e) {
		SystemLog() << "exception caught: "
			    << e;
		exit(-1);
	} catch (...) {
		SystemLog() << "illegal exception caught! terminating!";
		exit(-1);
	}

	return 0;
}

/* Revision history
 * $Log$
 * Revision 1.7  2004/06/08 19:58:04  dtrg
 * Fixed a bug where the address of incoming connections was thought to be the
 * address of *this* end of the connection, not the other end. In the process,
 * changed some this->blah instance variables to _blah.
 *
 * Revision 1.6  2004/05/30 01:55:13  dtrg
 * Numerous and major alterations to implement a system for processing more than
 * one message at a time, based around coroutines. Fairly hefty rearrangement of
 * constructors and object ownership semantics. Assorted other structural
 * modifications.
 *
 * Revision 1.5  2004/05/14 22:01:39  dtrg
 * Added inetd mode, where one message is processed from stdin and then spey
 * exits. Also added proper daemon functionality where spey detaches itself
 * cleanly from the console to go into the background.
 *
 * Revision 1.4  2004/05/13 23:33:28  dtrg
 * Discovered that I hadn't actually checked the fix in for that last bug! Also
 * noticed that the fix sorts out another problem where spey would terminate if a
 * connection to the internal mail server failed. Now it'll produce a diagnostic,
 * close the incoming connection, and start waiting again.
 *
 * Revision 1.3  2004/05/13 14:26:31  dtrg
 * Finally tracked down the annoying SQL-related crash. It seems that VACUUM is
 * not thread-safe and is causing the database session to expire. The correct
 * solution is not to use VACUUM, so I'm modifying speyctl to do that. However,
 * I'm leaving the recovery code in because of general resiliency issues, but
 * upgrading the diagnostic to a System-level message because it is actually
 * causing a message to be rejected when it might not be.
 *
 * Revision 1.2  2004/05/09 18:23:16  dtrg
 * SQL server now accessed asynchronously; backed out fix for mysterious SQL crash
 * and instead put in some code that should recover sanely from it. Don't know
 * what's going on here.
 *
 * Revision 1.1  2004/05/01 12:20:20  dtrg
 * Initial version.
 */
