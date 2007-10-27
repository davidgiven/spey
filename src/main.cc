/* version.cc
 * Contains the version number constants.
 *
 * Copyright (C) 2007 David Given
 * You may distribute under the terms of the GNU General Public
 * License version 2 as specified in the file COPYING that comes with the
 * Spey distribution.
 *
 * $Source$
 * $State$
 */

#include "spey.h"
#include <unistd.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>
#include <signal.h>
#include <iostream>
#include <fstream>

/* If Electric Fence is turned on, enable its maximum paranoia settings. */

#ifdef ELECTRICFENCE
bool EF_FREE_WIPES = true;
#endif

/* GNUTLS needs special code if it is to be used in multithreaded mode.
 * This macro emits it. */

#ifdef GNUTLS
GCRY_THREAD_OPTION_PTHREAD_IMPL;
#endif

SQL Sql;
SocketAddress FromAddress;
SocketAddress ToAddress;

static void drop_root_privileges()
{
	/* Don't do anything if we're not root. */

	if (getuid() != 0)
	{
		DetailLog() << "Not running as root, so not dropping any privileges.";
		return;
	}

	/* Don't do anything if dropping root privileges has been disabled. */

	string usergroupid = Settings::runtimeuserid();
	if (usergroupid == "")
	{
		DetailLog() << "Not dropping root privileges --- running as root";
		return;
	}

	/* Drop privileges. */

	{
		string::size_type seperator = usergroupid.find(':');
		if (seperator == string::npos)
			goto abort;
			
		/* Extract the user and group name from the string. */
		
		string username = usergroupid.substr(0, seperator);
		string groupname = usergroupid.substr(seperator+1);
		DetailLog() << "Dropping root privileges; running as "
		            << username
		            << ":"
		            << groupname;
	
		/* Look up the passwd and group fields. */
	
		uid_t userid = 0;
		gid_t groupid = 0;
		
		struct passwd* pwd = getpwnam(username.c_str());
		if (pwd)
		{
			userid = pwd->pw_uid;
			endpwent();
		}
			
		struct group* grp = getgrnam(groupname.c_str());
		if (grp)
		{
			groupid = grp->gr_gid;
			endgrent();
		}
		
		if (!pwd || !grp)
			goto abort;
	
		/* Drop privileges. */
		
		int result = setgid(groupid);
		result |= setuid(userid);
		if (result)
			goto abort;
			
		return;
	}
	
abort:
		throw Exception("Unable to drop root privileges --- terminating");
}

/* Everything in here must happen after any possible call to daemon().
 * daemon() does a fork(), which causes nasty things to happen if there's a
 * locked mutex. */

static void initialise(CLI& cli)
{
	Threadlet::initialise();
	Sql.open(cli.d());
	FromAddress.set(cli.f());
	ToAddress.set(cli.t());

#ifdef GNUTLS
	gcry_control(GCRYCTL_SET_THREAD_CBS, &gcry_threads_pthread);
	gnutls_global_init();
#endif
}

static int daemonmode(CLI& cli)
{
	// Detach from the console.
	if (!cli.x())
	{
		daemon(0, 0);
		Logger::detach();
	}
	 	
	DetailLog() << "------------- STARTUP ----------------";
	DetailLog() << "Spey version " << MajorVersion << " build " << BuildCount;

	initialise(cli);
	
	SystemLog() << "listening on "
		    << FromAddress;

	/* Create server thread. */
	(void) new ServerProcessor();

	ofstream("/var/run/spey.pid") << getpid() << endl;

	drop_root_privileges();

	Threadlet::halt();
	return 0;
}

static int inetdmode(CLI& cli)
{
	DetailLog() << "------------- INETD STARTUP ----------------";
	DetailLog() << "Spey version " << MajorVersion << " build " << BuildCount;

	initialise(cli);
	
	try {
		Settings::reload();
		SocketAddress dummyaddress;

		/* Create message thread. */
		(void) new MessageProcessor(0, dummyaddress);
		drop_root_privileges();

		Threadlet::halt();
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
		SystemLog() << "string exception caught: "
			    << e;
		exit(-1);
	} catch (Exception e) {
		SystemLog() << "generic exception caught: "
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
 * Revision 1.14  2007/10/24 22:51:30  dtrg
 * Finally fixed the horrible irreproducable mutex lockup bug.
 *
 * Revision 1.13  2007/04/18 22:26:56  dtrg
 * Fixed a bug where we were forgetting to tell gnutls that we were a
 * multithreaded application, resulting in it stepping on gcrypt's toes
 * and crashing at irregular intervals.
 *
 * Revision 1.12  2007/02/10 00:24:35  dtrg
 * Added support for TLS connections using the GNUTLS library. A X509
 * certificate and private key must be supplied for most purposes, but if they
 * are not provided anonymous authentication will be used instead (which
 * apparently only GNUTLS supports). Split the relay check up into two
 * separate parts; the trustedhosts table now specifies machines that can be
 * trusted to play nice, and can do relaying and be allowed to bypass the
 * greylisting; and allowedrecipients, which specifies what email address we're
 * expecting to receive. Also fixed some remaining niggles in the AUTH
 * proxy support, but this remains largely untested.
 *
 * Revision 1.11  2007/01/29 23:05:12  dtrg
 * Due to various unpleasant incompatibilities with ucontext, the
 * entire coroutine implementation has been rewritten to use
 * pthreads instead of user-level scheduling. This should make
 * things far more robust and portable, if a bit more heavyweight.
 * It also has the side effect of drastically simplified threadlet code.
 *
 * Revision 1.10  2005/10/31 22:20:36  dtrg
 * Added support for compiling with the Electric Fence memory debugger.
 *
 * Revision 1.9  2005/09/30 23:18:16  dtrg
 * Added support for dropping root privileges, by setting the runtime-user-id configuration variable to the desired user and group.
 *
 * Revision 1.8  2004/11/18 17:57:20  dtrg
 * Rewrote logging system so that it no longer tries to subclass stringstream,
 * that was producing bizarre results on gcc 3.3. Added version tracking to the
 * makefile; spey now knows what version and build number it is, and displays the
 * information in the startup banner. Now properly ignores SIGPIPE, which was
 * causing intermittent silent aborts.
 *
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
