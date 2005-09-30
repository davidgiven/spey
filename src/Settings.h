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
	static string runtimeuserid() { return _runtimeuserid; }
	
	static bool testrelay(const SocketAddress& sender,
			const string& recipient);
	
protected:
	static string _identity;
	static int _intolerant;
	static int _quarantinetime;
	static int _sockettimeout;
	static string _runtimeuserid;
};

#endif

/* Revision history
 * $Log$
 * Revision 1.2  2004/05/14 23:11:44  dtrg
 * Added decent relaying support. Also converted SocketAddress to use references a
 * lot rather than pass-by-value, out of general tidiness and the hope that it
 * will improve performance a bit.
 *
 * Revision 1.1  2004/05/01 12:20:20  dtrg
 * Initial version.
 *
 */
