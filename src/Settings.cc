/* Settings.cc
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

#include "spey.h"

string Settings::_identity;
int Settings::_intolerant;
int Settings::_quarantinetime;
int Settings::_sockettimeout;
int Settings::_greetpause;
string Settings::_runtimeuserid;
string Settings::_tlscertificatefile;
string Settings::_tlsprivatekeyfile;
bool Settings::_externaltls;
bool Settings::_externalauth;

string Settings::get(string key)
{
	stringstream s;
	s << "SELECT value FROM settings WHERE "
		"key = '"
	  << key
	  << "';";

	try
	{
		SQLQuery q(Sql, s.str());
		q.step();
		return q.getstring(0);
	}
	catch (SQLException e)
	{
		return "";
	}
}

void Settings::reload()
{
	_identity = get("identity");
	_intolerant = atoi(get("intolerant").c_str());
	_quarantinetime = atoi(get("quarantine-time").c_str());
	_sockettimeout = atoi(get("socket-timeout").c_str());
	_greetpause = atoi(get("greet-pause").c_str());
	_runtimeuserid = get("runtime-user-id");
	_tlscertificatefile = get("tls-certificate-file");
	_tlsprivatekeyfile = get("tls-private-key-file");
	_externaltls = atoi(get("external-tls").c_str());
	_externalauth = atoi(get("external-auth").c_str());
}

/* Check to see whether the machine that's connected to us is a trusted site.
 */
 
bool Settings::testtrusted(const SocketAddress& sender)
{
	stringstream s;
	s << "SELECT COUNT(*) FROM trustedhosts WHERE "
	  << "(" << (int)sender << " >> (32-right)) = (left >> (32-right));";

	{
		SQLQuery q(Sql, s.str());
		q.step();
		if (q.getint(0))
			return true;
	}

	return false;
}

/* Test to see if we want to accept mail to a particular address. */

bool Settings::testacceptance(const string& recipient)
{
	int i = recipient.find('@');
	string address = recipient.substr(0, i);
	string domain = recipient.substr(i+1);

	stringstream s;
	s << "SELECT COUNT(*) FROM validrecipients WHERE "
		<< "((left = '') OR (left = '" << address << "')) AND "
		<< "((right = '') OR (right = '" << domain << "'));";
		
	{
		SQLQuery q(Sql, s.str());
		q.step();
		if (q.getint(0))
			return true;
	}

	return false;
}

/* Revision history
 * $Log$
 * Revision 1.7  2007/02/10 00:24:35  dtrg
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
 * Revision 1.6  2007/02/01 18:41:49  dtrg
 * Reworked the SMTP AUTH code so that spey automatically figures out what
 * authentication mechanisms there are by asking the downstream server. The
 * external-auth setting variable is now a boolean. Rearranged various
 * other bits of code and fixed a lot of problems with the man pages.
 *
 * Revision 1.5  2007/01/31 12:58:25  dtrg
 * Added basic support for upstream AUTH requests based on Juan José
 * Gutiérrez de Quevedoo (juanjo@iteisa.com's patch. AUTH requests are
 * proxied through to the downstream server. Parts of the code still need a
 * rethink but it should all work.
 *
 * Revision 1.4  2007/01/29 00:37:48  dtrg
 * Removed some stray stdout tracing that got checked in by accident.
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
 */

