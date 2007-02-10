/* Statistics.h
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

#ifndef STATISTICS_H
#define STATISTICS_H

struct Statistics {
	static void malformedDomain();
	static void malformedAddress();
	static void illegalRelaying();
	static void spokeTooSoon();
	static void timeout();
	static void greylisted();
	static void accepted();
	static void blacklisted();
	static void whitelisted();
	static void blackholed();

protected:
	static void count(string which);
};

#endif

/* Revision history
 * $Log$
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
