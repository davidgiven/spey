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
	static string tlscertificatefile() { return _tlscertificatefile; }
	static string tlsprivatekeyfile() { return _tlsprivatekeyfile; }
	static bool externaltls() { return _externaltls; }
	static bool externalauth() { return _externalauth; }
	
	static bool testtrusted(const SocketAddress& sender);
	static bool testacceptance(const string& recipient);
	
protected:
	static string _identity;
	static int _intolerant;
	static int _quarantinetime;
	static int _sockettimeout;
	static string _runtimeuserid;
	static string _tlscertificatefile;
	static string _tlsprivatekeyfile;
	static bool _externaltls;
	static bool _externalauth;
};

#endif

/* Revision history
 * $Log$
 * Revision 1.5  2007/02/01 18:41:49  dtrg
 * Reworked the SMTP AUTH code so that spey automatically figures out what
 * authentication mechanisms there are by asking the downstream server. The
 * external-auth setting variable is now a boolean. Rearranged various
 * other bits of code and fixed a lot of problems with the man pages.
 *
 * Revision 1.4  2007/01/31 12:58:25  dtrg
 * Added basic support for upstream AUTH requests based on Juan José
 * Gutiérrez de Quevedoo (juanjo@iteisa.com's patch. AUTH requests are
 * proxied through to the downstream server. Parts of the code still need a
 * rethink but it should all work.
 *
 * Revision 1.3  2005/09/30 23:18:16  dtrg
 * Added support for dropping root privileges, by setting the runtime-user-id configuration variable to the desired user and group.
 *
 * Revision 1.2  2004/05/14 23:11:44  dtrg
 * Added decent relaying support. Also converted SocketAddress to use references a
 * lot rather than pass-by-value, out of general tidiness and the hope that it
 * will improve performance a bit.
 *
 * Revision 1.1  2004/05/01 12:20:20  dtrg
 * Initial version.
 *
 */
