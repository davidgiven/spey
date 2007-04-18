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

/* Revision history
 * $Log$
 * Revision 1.4  2007/02/10 20:59:16  dtrg
 * Added support for DNS-based RBLs.
 *
 * Revision 1.3  2007/02/10 19:46:44  dtrg
 * Added greet-pause support. Moved the trusted hosts check to right after
 * connection so that greet-pause doesn't apply to trusted hosts. Fixed a bug
 * in the AUTH supported that meant that authenticated connections had no
 * extra privileges (oops). Added the ability to reset all statistics on demand.
 *
 * Revision 1.2  2004/06/21 23:12:46  dtrg
 * Added blacklisting and whitelisting support.
 *
 * Revision 1.1  2004/05/01 12:20:20  dtrg
 * Initial version.
 */
