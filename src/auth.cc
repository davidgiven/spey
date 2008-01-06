/* auth.cc
 * Authentication conversation algorithms.
 *
 * Copyright (C) 2008 David Given
 * You may distribute under the terms of the GNU General Public
 * License version 2 as specified in the file COPYING that comes with the
 * Spey distribution.
 *
 * $Source$
 * $State$
 */

#include "spey.h"

static string conversation(Socket& stream, const string& message)
{
	SMTPResponse response(334, message);
	stream.writeline(response);
	
	string s = stream.readline();
	if (s == "*")
		throw AuthenticationCancelledException();
	return s;
}

static int check_credentials(const char* type, const char* domain,
		const char* username, const char* password)
{
	ParseLog() << "AUTH PLAIN: domain="
	           << domain
	           << " username="
	           << username
	           << " password="
	           << password;

	try
	{
		SQLQuery q(Sql, "SELECT COUNT(*) FROM users WHERE "
		                  "username=%Q AND password=%Q;",
		                  username, password);
		q.step();
		if (q.getint(0) == 0)
			return 535;
	}
	catch (SQLException e)
	{
		return 535;
	}
	
	return 235;
}

int auth_plain(Socket& stream, const string& initial)
{
	/* Retrieve the credentials. */
	
	string s = initial;
	if (s == "")
		s = conversation(stream, "");
	
	/* Decode them, if possible. */
	
	char buffer[s.length() + 1];
	int len = base64_decode(s, buffer);
	if (len == -1)
		return 535;
	buffer[len] = '\0';
	
	/* Ensure there are exactly two \0 bytes in the decoded buffer ---
	 * sanity check. */
	
	{
		int zeroes = 0;
		for (int i=0; i<len; i++)
			if (buffer[i] == '\0')
				zeroes++;
		
		if (zeroes != 2)
			return 535;
	}
	
	/* Parse them. */
	
	const char* domain = buffer;
	const char* username = domain;
	while (*username++)
		;
	
	const char* password = username;
	while (*password++)
		;

	return check_credentials("PLAIN", domain, username, password);
}

int auth_login(Socket& stream, const string& initial)
{
	if (initial != "")
		return 535;
	
	/* Retrieve the credentials. */
	
	/* 'VXNlcm5hbWU6' = 'Username:' */
	string username64 = conversation(stream, "VXNlcm5hbWU6");
	
	/* 'UGFzc3dvcmQ6' = 'Password:' */
	string password64 = conversation(stream, "UGFzc3dvcmQ6");
	
	/* Decode. */
	
	int i;
	char username[username64.length() + 1];
	i = base64_decode(username64, username);
	if (i == -1)
		return 535;
	username[i] = '\0';
	
	char password[password64.length() + 1];
	i = base64_decode(password64, password);
	if (i == -1)
		return 535;
	password[i] = '\0';
	
	return check_credentials("LOGIN", "", username, password);
}
