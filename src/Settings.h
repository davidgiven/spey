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
	static bool externalauth() { return _externalauth; }
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
	static bool _externalauth;
	static string _rbllist;
};

#endif

/* Revision history
 * $Log$
 * Revision 1.9  2007/10/24 20:44:15  dtrg
 * Did a lot of minor code cleanups and C++ style improvements: uncopyable C++
 * objects are now marked as such and do not have copy constructors, and RAI is
 * used for the threadlet mutex.
 *
 * Revision 1.8  2007/02/10 20:59:16  dtrg
 * Added support for DNS-based RBLs.
 *
 * Revision 1.7  2007/02/10 19:46:44  dtrg
 * Added greet-pause support. Moved the trusted hosts check to right after
 * connection so that greet-pause doesn't apply to trusted hosts. Fixed a bug
 * in the AUTH supported that meant that authenticated connections had no
 * extra privileges (oops). Added the ability to reset all statistics on demand.
 *
 * Revision 1.6  2007/02/10 00:24:35  dtrg
 * Added support for TLS connections using the GNUTLS library. A X509
 * certificate and private key must be supplied for most purposes, but if they
 * are not provided anonymous authentication will be used instead (which
 * apparently only GNUTLS supports). Split the relay check up into two
 * separate parts; the trustedhosts table now specifies machines that can be
 * trusted to play nice, and can do relaying and be allowed to bypass the
 * greylisting; and allowedrecipients, which specifies what email address we're
 * expecting to receive. Also fixed some remaining niggles in the AUTH
 * proxy support, but this remains largely untested.
 *
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
