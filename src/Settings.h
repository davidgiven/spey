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

struct Settings : uncopyable
{
	static string get(string key);
	static void reload();

	static string identity() { return _identity; }
	static int intolerant() { return _intolerant; }
	static int quarantinetime() { return _quarantinetime; }
	static int sockettimeout() { return _sockettimeout; }
	static int greetpause() { return _greetpause; }
	static string runtimeuserid() { return _runtimeuserid; }
	static string tlscertificatefile() { return _tlscertificatefile; }
	static string tlsprivatekeyfile() { return _tlsprivatekeyfile; }
	static bool externaltls() { return _externaltls; }
	static AuthMode externalauthmode() { return _externalauthmode; }
	static string rbllist() { return _rbllist; };
	
	static bool testtrusted(const SocketAddress& sender);
	static bool testacceptance(const string& recipient);
	
protected:
	static string _identity;
	static int _intolerant;
	static int _quarantinetime;
	static int _sockettimeout;
	static int _greetpause;
	static string _runtimeuserid;
	static string _tlscertificatefile;
	static string _tlsprivatekeyfile;
	static bool _externaltls;
	static AuthMode _externalauthmode;
	static string _rbllist;
};

#endif
