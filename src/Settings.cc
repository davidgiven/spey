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

#include "common.h"

string Settings::_identity;
int Settings::_intolerant;
int Settings::_quarantinetime;
int Settings::_sockettimeout;
int Settings::_greetpause;
string Settings::_runtimeuserid;
string Settings::_tlscertificatefile;
string Settings::_tlsprivatekeyfile;
bool Settings::_externaltls;
AuthMode Settings::_externalauthmode;
string Settings::_rbllist;

string Settings::get(string key)
{
	try
	{
		SQLQuery q(Sql, "SELECT value FROM settings WHERE "
		                  "key=%Q;",
		                  key.c_str());
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
	_rbllist = get("rbl-list");
	
	string externalauthmode = get("external-auth-mode");
	if (externalauthmode == "proxy")
		_externalauthmode = ProxyAuth;
	else if (externalauthmode == "internal")
		_externalauthmode = InternalAuth;
	else if (externalauthmode == "none")
		_externalauthmode = NoAuth;
	else
	{
		_externalauthmode = NoAuth;
		SystemLog() << "'"
		            << externalauthmode
		            << "' is not a valid value for the 'external-auth-mode' "
		            << "setting; disabling external AUTH";
	}		
}

/* Check to see whether the machine that's connected to us is a trusted site.
 */
 
bool Settings::testtrusted(const SocketAddress& sender)
{
	{
		SQLQuery q(Sql, "SELECT COUNT(*) FROM trustedhosts WHERE "
		                  "(%u >> (32-right)) = (left >> (32-right));",
		                  (int)sender);
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

	{
		SQLQuery q(Sql, "SELECT COUNT(*) FROM validrecipients WHERE "
		                  "((left = '') OR (left = %Q)) AND "
		                  "((right = '') OR (right = %Q));",
		                  address.c_str(), domain.c_str()); 
		q.step();
		if (q.getint(0))
			return true;
	}

	return false;
}
