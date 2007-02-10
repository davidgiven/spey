/* Statistics.cc
 * Interface to the statistics stored in the SQL database.
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

void Statistics::malformedDomain()
{
	count("malformed-domain");
}

void Statistics::malformedAddress()
{
	count("malformed-address");
}

void Statistics::spokeTooSoon()
{
	count("spoke-too-soon");
}

void Statistics::timeout()
{
	count("timeout");
}

void Statistics::illegalRelaying()
{
	count("illegal-relay");
}

void Statistics::greylisted()
{
	count("greylisted");
}

void Statistics::accepted()
{
	count("accepted");
}

void Statistics::blacklisted()
{
	count("blacklisted");
}

void Statistics::whitelisted()
{
	count("whitelisted");
}

void Statistics::count(string name)
{
	stringstream s;
	s << "UPDATE statistics SET value=value+1 WHERE key='"
	  << name
	  << "';";

	SQLQuery q(Sql, s.str());
	q.step();
}

/* Revision history
 * $Log$
 * Revision 1.2  2004/06/21 23:12:46  dtrg
 * Added blacklisting and whitelisting support.
 *
 * Revision 1.1  2004/05/01 12:20:20  dtrg
 * Initial version.
 */
