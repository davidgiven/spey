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
string Settings::_runtimeuserid;

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
	_runtimeuserid = get("runtime-user-id");
}

bool Settings::testrelay(const SocketAddress& sender, const string& recipient)
{
	stringstream s;
	s << "SELECT COUNT(*) FROM allowrelayingfrom WHERE "
	  << "(" << (int)sender << " >> (32-right)) = (left >> (32-right));";

	{
		SQLQuery q(Sql, s.str());
		q.step();
		if (q.getint(0))
			return 1;
	}

	int i = recipient.find('@');
	string address = recipient.substr(0, i);
	string domain = recipient.substr(i+1);

	s.str("");
	s << "SELECT COUNT(*) FROM allowrelayingto WHERE "
		<< "((left = '') OR (left = '" << address << "')) AND "
		<< "((right = '') OR (right = '" << domain << "'));";
	cout << s.str() << endl;
		
	{
		SQLQuery q(Sql, s.str());
		q.step();
		if (q.getint(0))
			return 1;
	}

	return 0;
}

/* Revision history
 * $Log$
 * Revision 1.2  2004/05/14 23:11:44  dtrg
 * Added decent relaying support. Also converted SocketAddress to use references a
 * lot rather than pass-by-value, out of general tidiness and the hope that it
 * will improve performance a bit.
 *
 * Revision 1.1  2004/05/01 12:20:20  dtrg
 * Initial version.
 */

