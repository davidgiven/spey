/* Settings.h
 * Interface the configuration settings in the database.
 *
 * Copyright (C) 2004 David Given
 * You may distribute under the terms of the GNU General Public
 * License version 2 as specified in the file COPYING that comes with the
 * Spey distribution.
 *
 * $Source$
 * $State$
 */

#ifndef SETTINGS_H
#define SETTINGS_H

struct Settings {
	static string get(string key);
	static void reload();

	static string identity() { return _identity; }
	static int intolerant() { return _intolerant; }
	static int quarantinetime() { return _quarantinetime; }
	static int sockettimeout() { return _sockettimeout; }
	
	static bool testrelay(const SocketAddress& sender,
			const string& recipient);
	
protected:
	static string _identity;
	static int _intolerant;
	static int _quarantinetime;
	static int _sockettimeout;
};

#endif

/* Revision history
 * $Log$
 * Revision 1.1  2004/05/01 12:20:20  dtrg
 * Initial version.
 *
 */
