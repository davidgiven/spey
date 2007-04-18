/* greylist.cc
 * Main greylisting engine.
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
#include <sys/time.h>

GreylistResponse greylist(uint32_t sender, string fromaddress,
		string toaddress)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	long lastseen = tv.tv_sec;
	long firstseen = 0;
	int timesseen = 0;
	bool failed = 1;

	if (fromaddress == "")
		fromaddress = "(probe)";

	DetailLog() << "greylist check: "
		    << sender
		    << ", "
		    << fromaddress
		    << ", "
		    << toaddress
		    << " at "
		    << tv.tv_sec;

	/* First check the whitelist. */

	{
		SQLQuery q(Sql, "SELECT COUNT(*) FROM whitelist WHERE "
		                  "(%Q LIKE sender) AND (%Q LIKE recipient);",
		                  fromaddress.c_str(), toaddress.c_str());
		if (!q.step())
			goto notfound;
		if (q.getint(0))
		{
			DetailLog() << "matches whitelist";
			return Accepted;
		}
	}

	/* Then check the blacklist. */

	{
		SQLQuery q(Sql, "SELECT COUNT(*) FROM blacklist WHERE "
		                   "(%Q LIKE sender) AND (%Q LIKE recipient);",
		                   fromaddress.c_str(), toaddress.c_str());
		if (!q.step())
			goto notfound;
		if (q.getint(0))
		{
			DetailLog() << "matches blacklist";
			return BlackListed;
		}
	}

	{
		SQLQuery q(Sql, "SELECT firstseen, timeseen FROM triples WHERE "
		                  "(sender=%u) AND (fromaddress=%Q) AND (toaddress=%Q);",
		                  sender, fromaddress.c_str(), toaddress.c_str());
		if (!q.step())
			goto notfound;
		firstseen = q.getint(0);
		timesseen = q.getint(1);
	}

	/* On this data path, we've seen this particular triple before, so we
	 * check the time, do the greylist test and update the record. */

	if ((lastseen - firstseen) > Settings::quarantinetime())
	{
		failed = 0;
		timesseen++;
	}

	{
		SQLQuery q(Sql, "UPDATE triples SET "
		                  "lastseen=%ld, timesseen=%d, WHERE "
		                  "(sender=%u) AND (fromaddress=%Q) AND (toaddress=%Q);",
		                  lastseen, timesseen, sender, fromaddress.c_str(), toaddress.c_str());
		q.step();
	}

	return failed ? GreyListed : Accepted;

notfound:
	/* This is the first time we've seen this triple. Add a record, but
	 * fail it. */

	{
		SQLQuery q(Sql, "INSERT INTO triples VALUES "
		                  "(NULL, %u, %Q, %Q, 1, %ld, %ld);",
		                  sender, fromaddress.c_str(), toaddress.c_str(), lastseen, lastseen);
		q.step();
	}

	return GreyListed;
}

/* Revision history
 * $Log$
 * Revision 1.8  2007/02/10 20:59:16  dtrg
 * Added support for DNS-based RBLs.
 *
 * Revision 1.7  2007/01/29 23:01:19  dtrg
 * Fixed a compiler warning.
 *
 * Revision 1.6  2006/04/26 21:56:23  dtrg
 * Backed out the previous change, as it wasn't necessary (SMTPCommand's parser
 * already converted the addresses to lower case).
 *
 * Revision 1.5  2006/04/25 20:07:59  dtrg
 * Changed the greylister so that it converts email addresses to lower case before trying to match
 * them. Email address are supposed to be case insensitive (in ASCII), but nobody had ever tried
 * it until now...
 *
 * Revision 1.4  2004/11/18 17:57:20  dtrg
 * Rewrote logging system so that it no longer tries to subclass stringstream,
 * that was producing bizarre results on gcc 3.3. Added version tracking to the
 * makefile; spey now knows what version and build number it is, and displays the
 * information in the startup banner. Now properly ignores SIGPIPE, which was
 * causing intermittent silent aborts.
 *
 * Revision 1.3  2004/06/22 10:05:37  dtrg
 * Fixed some more logic flow bugs in the blacklist code. (Blacklisted messages
 * were being reported as greylisted.)
 *
 * Revision 1.2  2004/06/21 23:12:47  dtrg
 * Added blacklisting and whitelisting support.
 *
 * Revision 1.1  2004/05/01 12:20:20  dtrg
 * Initial version.
 */
