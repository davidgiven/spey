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

GreylistResponse greylist(unsigned int sender, string fromaddress, string toaddress)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	long lastseen = tv.tv_sec;
	long firstseen;
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
		stringstream s;
		s << "SELECT COUNT(*) FROM whitelist WHERE "
		  << "('"
		  << fromaddress
		  << "' LIKE sender) AND ('"
		  << toaddress
		  << "' LIKE recipient);";

		SQLQuery q(Sql, s.str());
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
		stringstream s;
		s << "SELECT COUNT(*) FROM blacklist WHERE "
		  << "('"
		  << fromaddress
		  << "' LIKE sender) AND ('"
		  << toaddress
		  << "' LIKE recipient);";

		SQLQuery q(Sql, s.str());
		if (!q.step())
			goto notfound;
		if (q.getint(0))
		{
			DetailLog() << "matches blacklist";
			return BlackListed;
		}
	}

	{
		stringstream s;
		s << "SELECT firstseen, timesseen FROM triples WHERE "
		  << "(sender="
		  << sender
		  << ") AND (fromaddress='"
		  << fromaddress
		  << "') AND (toaddress='"
		  << toaddress
		  << "');";

		SQLQuery q(Sql, s.str());
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
		stringstream s;
		s << "UPDATE triples SET "
		  << "lastseen="
		  << lastseen
		  << ", timesseen="
		  << timesseen
		  << " WHERE "
		  << "(sender="
		  << sender
		  << ") AND (fromaddress='"
		  << fromaddress
		  << "') AND (toaddress='"
		  << toaddress
		  << "');";

		SQLQuery q(Sql, s.str());
		q.step();
	}

	return failed ? GreyListed : Accepted;

notfound:
	/* This is the first time we've seen this triple. Add a record, but
	 * fail it. */

	{
		stringstream s;
		s << "INSERT INTO triples VALUES (NULL, "
		  << sender
		  << ", '"
		  << fromaddress
		  << "', '"
		  << toaddress
		  << "', 1, "
		  << lastseen
		  << ", "
		  << lastseen
		  << ");";

		SQLQuery q(Sql, s.str());
		q.step();
	}

	return GreyListed;
}

/* Revision history
 * $Log$
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
