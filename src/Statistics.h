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

protected:
	static void count(string which);
};

#endif

/* Revision history
 * $Log$
 * Revision 1.2  2004/06/21 23:12:46  dtrg
 * Added blacklisting and whitelisting support.
 *
 * Revision 1.1  2004/05/01 12:20:20  dtrg
 * Initial version.
 */
