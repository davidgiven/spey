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
			Socket s = server.accept();
			Settings::reload();
			MessageProcessor mp(s);
			try {
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
 */