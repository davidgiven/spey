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
	static void timeout();
	static void greylisted();
	static void accepted();

protected:
	static void count(string which);
};

#endif

/* Revision history
 * $Log$
 */