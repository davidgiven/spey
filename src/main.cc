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

int main(int argc, char* argv[])
{

	try {
		CLI cli(argc, argv);
		Logger::setlevel(cli.v());
		
		int pid = getpid();
		SystemLog() << "Spey starting on pid "
			    << pid
			    << flush;
		DetailLog() << "------------- STARTUP ----------------"
			    << flush;

		ofstream("/var/run/spey.pid") << pid << endl;

		Sql.open(cli.d());
		FromAddress.set(cli.f());
		ToAddress.set(cli.t());

		SystemLog() << "listening on "
			    << FromAddress
			    << flush;

		SocketServer server(FromAddress);

		for (;;)
		{
			MessageLog() << "waiting for connection"
				     << flush;
			try {
				Socket s = server.accept();
				Settings::reload();
				MessageProcessor mp(s);
				mp.process();
			} catch (NetworkTimeoutException e) {
				Statistics::timeout();
				MessageLog() << "Socket timeout; aborting"
					     << flush;
			} catch (NetworkException e) {
				MessageLog() << "exception caught: "
					     << e
					     << flush;
				MessageLog() << "message processing aborted"
					     << flush;
			} catch (SQLException e) {
				SystemLog() << "SQL error: "
				             << e
					     << flush;
				SystemLog() << "attempting to recover"
					     << flush;
				Sql.close();
				Sql.open(cli.d());
			}
		}

	} catch (string e) {
		SystemLog() << "exception caught: "
			    << e
			    << flush;
		exit(-1);
	} catch (Exception e) {
		SystemLog() << "exception caught: "
			    << e
			    << flush;
		exit(-1);
	}

	return 0;
}

/* Revision history
 * $Log$
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
 *
 */
