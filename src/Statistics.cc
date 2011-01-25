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

#include "common.h"

void Statistics::malformedDomain()     { count("malformed-domain"); }
void Statistics::malformedAddress()    { count("malformed-address"); }
void Statistics::spokeTooSoon()        { count("spoke-too-soon"); }
void Statistics::timeout()             { count("timeout"); }
void Statistics::illegalRelaying()     { count("illegal-relay"); }
void Statistics::greylisted()          { count("greylisted"); }
void Statistics::accepted()            { count("accepted"); }
void Statistics::blacklisted()         { count("blacklisted"); }
void Statistics::whitelisted()         { count("whitelisted"); }
void Statistics::blackholed()          { count("blackholed"); }

void Statistics::count(string name)
{
	SQLQuery q(Sql, "UPDATE statistics SET value=value+1 WHERE "
	                  "key=%Q;",
	                  name.c_str());
	q.step();
}
