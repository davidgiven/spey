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

bool greylist(unsigned int sender, string fromaddress, string toaddress)
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
		    << tv.tv_sec
		    << flush;

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

	return failed;

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

	return 1;
}

/* Revision history
 * $Log$
 */