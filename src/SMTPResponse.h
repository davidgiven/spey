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

struct SMTPResponse {
	SMTPResponse();
	SMTPResponse(SMTPResponse& r);
	SMTPResponse(int code, string parameter="");
	SMTPResponse(Socket& in);

	void set();
	void set(int code, string parameter="");
	void set(Socket& in);
	void parmoverride(string parameter);
	void msgoverride(string message);

	void check(int code, string message);
	bool issuccess();
	bool isretry();
	bool iserror();

	operator string ();
	string arg() { return _parameter; }

protected:
	int _code;
	string _parameter;
	string _msgoverride;
};

inline ostream& operator << (ostream& s, SMTPResponse& sa)
{
	s << (string) sa;
	return s;
}

#endif

/* Revision history
 * $Log$
 */