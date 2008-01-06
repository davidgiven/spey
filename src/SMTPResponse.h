/* SMTPResponse.h
 * Represents a status response from the local SMTP server.
 *
 * Copyright (C) 2004 David Given
 * You may distribute under the terms of the GNU General Public
 * License version 2 as specified in the file COPYING that comes with the
 * Spey distribution.
 *
 * $Source$
 * $State$
 */

#ifndef SMTPRESPONSE_H
#define SMTPRESPONSE_H

#include <vector>

struct SMTPResponse
{
	SMTPResponse();
	SMTPResponse(SMTPResponse& r);
	SMTPResponse(int code, string parameter="");
	SMTPResponse(Socket& in);

	void set();
	void set(int code, string parameter="");
	void set(Socket& in);
	
	void continuationoverride();
	void continuationoverride(vector<string>& continuation);
	
	void parmoverride(string parameter);
	void msgoverride(string message);

	void check(int code, string message);
	bool issuccess();
	bool isretry();
	bool iserror();

	operator string ();
	string arg() { return _parameter; }
	int code() { return _code; }
	vector<string>& continuation() { return _continuation; }

protected:
	int _code;
	string _parameter;
	string _msgoverride;
	vector<string> _continuation;
};

inline Logger& operator << (Logger& s, SMTPResponse& sa)
{
	s << (string) sa;
	return s;
}

#endif
