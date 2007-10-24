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

/* Revision history
 * $Log$
 * Revision 1.6  2007/10/24 20:44:15  dtrg
 * Did a lot of minor code cleanups and C++ style improvements: uncopyable C++
 * objects are now marked as such and do not have copy constructors, and RAI is
 * used for the threadlet mutex.
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

