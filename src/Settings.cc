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

string Settings::get(string key)
{
	stringstream s;
	s << "SELECT value FROM settings WHERE "
		"key = '"
	  << key
	  << "';";

	SQLQuery q(Sql, s.str());
	q.step();
	return q.getstring(0);
}

void Settings::reload()
{
	_identity = get("identity");
	_intolerant = atoi(get("intolerant").c_str());
	_quarantinetime = atoi(get("quarantine-time").c_str());
	_sockettimeout = atoi(get("socket-timeout").c_str());
}

bool Settings::testrelay(string address)
{
	if (address == _identity)
		return 1;
	return 0;
}

/* Revision history
 * $Log$
 */