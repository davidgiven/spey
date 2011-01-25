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
		SQLQuery q(Sql, "SELECT firstseen, timesseen FROM triples WHERE "
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
		                  "lastseen=%ld, timesseen=%d WHERE "
		                  "(sender=%u) AND (fromaddress=%Q) AND (toaddress=%Q);",
		                  lastseen, timesseen, sender, fromaddress.c_str(), toaddress.c_str());
		q.step();
	}

	DetailLog() << (failed ? "message already seen, still greylisted" : "message accepted");
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

	DetailLog() << "new message seen, greylisted";
	return GreyListed;
}
