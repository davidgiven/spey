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
	
	/* Note: continuation support is currently very crude. Continuations are
	 * not read, must be set programmatically, and may consist of only one
	 * line. (Used by external-auth support.) */
	 
	void continuationoverride(string continuation);
	
	void parmoverride(string parameter);
	void msgoverride(string message);

	void check(int code, string message);
	bool issuccess();
	bool isretry();
	bool iserror();

	operator string ();
	string arg() { return _parameter; }
	int code() { return _code; }

protected:
	int _code;
	string _parameter;
	string _msgoverride;
	bool _hascontinuation;
	string _continuation;
};

inline Logger& operator << (Logger& s, SMTPResponse& sa)
{
	s << (string) sa;
	return s;
}

#endif

/* Revision history
 * $Log$
 * Revision 1.3  2004/11/18 17:57:20  dtrg
 * Rewrote logging system so that it no longer tries to subclass stringstream,
 * that was producing bizarre results on gcc 3.3. Added version tracking to the
 * makefile; spey now knows what version and build number it is, and displays the
 * information in the startup banner. Now properly ignores SIGPIPE, which was
 * causing intermittent silent aborts.
 *
 * Revision 1.2  2004/06/22 21:01:02  dtrg
 * Made a lot of minor tweaks so that spey now builds under gcc 3.3. (3.3 is a lot
 * closer to the C++ standard than 2.95 is; plus, the standard library is now
 * rather different, which means that I'm not allowed to do things like have local
 * variables called errno.)
 *
 * Revision 1.1  2004/05/01 12:20:20  dtrg
 * Initial version.
 */

