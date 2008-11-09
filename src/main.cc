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
		if (daemon(0, 0))
			throw Exception("Unable to daemonise --- terminating");
		Logger::detach();
	}

	DetailLog() << "------------- STARTUP ----------------";
	DetailLog() << "Spey version " << MajorVersion << " build " << BuildCount;

	initialise(cli);
	Settings::reload();

	SystemLog() << "listening on "
		    << FromAddress;

	/* Create server thread. */
	Threadlet* t = new ServerProcessor();
	t->start();

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
		Threadlet* t = new MessageProcessor(0, dummyaddress);
		drop_root_privileges();
		t->start();

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
